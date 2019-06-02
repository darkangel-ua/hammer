#include <boost/make_unique.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant/get.hpp>
#include <hammer/core/testing_generators.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/shared_lib_dirs_writer.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/cmdline_action.h>
#include <hammer/core/testing_fail_action.h>
#include <hammer/core/testing_run_meta_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/testing_build_environment.h>
#include <hammer/core/testing_compile_link_base_generator.h>

using std::unique_ptr;
using boost::make_unique;
using boost::make_shared;
namespace fs = boost::filesystem;
using std::string;

namespace hammer {
namespace {

struct testing_run_args_writer : public argument_writer {
   testing_run_args_writer(const type_registry& tr) : argument_writer("args"), runner_type_(tr.get(types::TESTING_RUN)) {}
   argument_writer* clone() const override { return new testing_run_args_writer(*this); }
   std::vector<const feature*> valuable_features() const override { return {}; }
   void write_impl(std::ostream& output,
                   const build_node& node,
                   const build_environment& environment) const override;

   const target_type& runner_type_;
};

void testing_run_args_writer::write_impl(std::ostream& output,
                                         const build_node& node,
                                         const build_environment& environment) const
{
   auto find_runner_mt = [&] {
      for (const basic_build_target* p: node.products_) {
         if (p->type().equal_or_derived_from(runner_type_))
            return dynamic_cast<const testing_run_meta_target*>(p->get_meta_target());
      }

      assert(false);
      throw std::runtime_error("Failed to find runner meta target");
   };

   const auto* mt = find_runner_mt();
   assert(mt);

   for (const auto& arg : mt->args_) {
      if (const string* id = boost::get<string>(&arg))
         output << '"' << *id << "\" ";
      else if (const fs::path* path = boost::get<fs::path>(&arg)) {
         if (path->has_root_path())
            output << '"' << path->string() << "\" ";
         else {
            auto l = mt->location() / *path;
            l.normalize();
            output << '"' << l.string() << "\" ";
         }
      } else
         throw std::logic_error("testing_run_args_writer::write_impl");
   }
}

void add_testing_suite_generator(engine& e,
                                 generator_registry& gr) {
   auto sources = make_consume_types(e, {
      types::TESTING_OUTPUT,
      types::TESTING_SUITE
   });

   auto products = make_product_types(e, {types::TESTING_SUITE});
   auto g = make_unique<generator>(e, "testing.suite", sources, products, true, build_action_ptr{});
   g->include_composite_generators(true);
   gr.insert(std::move(g));
}

class testing_action : public build_action {
   public:
      testing_action(engine& e,
                     build_action_ptr wrapped_action,
                     std::string name = {})
         : build_action(name.empty() ? wrapped_action->name() : name + "(" + wrapped_action->name() + ")"),
           wrapped_action_(std::move(wrapped_action)),
           output_arg_writer_("output", e.get_type_registry().get(types::TESTING_OUTPUT), product_argument_writer::output_strategy::FULL_PATH),
           passed_arg_writer_("passed", e.get_type_registry().get(types::TESTING_PASSED), product_argument_writer::output_strategy::FULL_PATH)
      {
      }

      std::string
      target_tag(const build_node& node,
                 const build_environment& environment) const override {
         return wrapped_action_->target_tag(node, environment);
      }

      std::vector<const feature*>
      valuable_features() const override { return wrapped_action_->valuable_features(); }

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override {
         testing_build_environment env{environment};
         const bool result = wrapped_action_->execute(node, env);
         if (!result)
            return result;

         std::ostringstream passed_filename;
         passed_arg_writer_.write(passed_filename, node, env);
         if (!env.write_tag_file(passed_filename.str(), "passed"))
            return false;

         std::ostringstream output_filename;
         output_arg_writer_.write(output_filename, node, env);
         return env.write_tag_file(output_filename.str(), env.output_stream_.str() + env.error_stream_.str());
      }

   private:
      build_action_ptr wrapped_action_;
      product_argument_writer output_arg_writer_;
      product_argument_writer passed_arg_writer_;
};

}

void add_testing_generators(engine& e,
                            generator_registry& gr)
{
   auto run_product = make_shared<product_argument_writer>("run_product", e.get_type_registry().get(types::TESTING_RUN));
   auto test_executable = make_shared<source_argument_writer>("test_executable", e.get_type_registry().get(types::EXE));
   auto additional_dirs = make_shared<shared_lib_dirs_writer>("additional_dirs", e.get_type_registry().get(types::SHARED_LIB));
   auto args = make_shared<testing_run_args_writer>(e.get_type_registry());
#if defined(_WIN32)
   cmdline_builder cmdline("@SET PATH=%PATH%;$(additional_dirs)\n"
                           "@$(test_executable) $(args)\n");
#else
   cmdline_builder cmdline("export LD_LIBRARY_PATH=$(additional_dirs):$LD_LIBRARY_PATH\n"
                           "$(test_executable) $(args)");
#endif
   cmdline += test_executable;
   cmdline += additional_dirs;
   cmdline += args;

   auto action = [&] {
      auto cmd_action = std::make_shared<cmdline_action>("testing.run", run_product);
      *cmd_action += cmdline;
      return std::make_shared<testing_action>(e, std::move(cmd_action));
   }();

   auto sources = make_consume_types(e, {types::EXE});
   auto products = make_product_types(e, {types::TESTING_RUN, types::TESTING_PASSED});

   unique_ptr<generator> g(new generator(e, "testing.run", sources, products, true, action));
   g->include_composite_generators(true);
   gr.insert(std::move(g));

   add_testing_suite_generator(e, gr);
}

void add_compile_generators(engine& e,
                            const build_action_ptr& compile_action,
                            std::function<std::unique_ptr<generator>(const build_action_ptr& action)> compile_generator_creator)
{
   e.generators().insert(compile_generator_creator(compile_action));

   auto a1 = std::make_shared<testing_action>(e, compile_action, "testing.compile");
   auto g1 = make_unique<testing_compile_link_base_generator>(e,
                                                              "testing.compile",
                                                              std::move(compile_generator_creator(a1)),
                                                              types::TESTING_COMPILE_SUCCESSFUL,
                                                              false);
   e.generators().insert(std::move(g1));

   auto a2 = std::make_shared<testing_action>(e, std::make_shared<testing_fail_action>(e, "testing.compile-fail", compile_action));
   auto g2 = make_unique<testing_compile_link_base_generator>(e,
                                                              "testing.compile-fail",
                                                              std::move(compile_generator_creator(a2)),
                                                              types::TESTING_COMPILE_FAIL,
                                                              true);
   e.generators().insert(std::move(g2));
}

void add_link_generators(engine& e,
                         const build_action_ptr& link_action,
                         std::function<std::unique_ptr<generator>(const build_action_ptr& action)> link_generator_creator)
{
   e.generators().insert(link_generator_creator(link_action));

   auto a1 = std::make_shared<testing_action>(e, link_action, "testing.link");
   auto g1 = make_unique<testing_compile_link_base_generator>(e,
                                                              "testing.link",
                                                              std::move(link_generator_creator(a1)),
                                                              types::TESTING_LINK_SUCCESSFUL,
                                                              false);
   e.generators().insert(std::move(g1));

   auto a2 = std::make_shared<testing_action>(e, std::make_shared<testing_fail_action>(e, "testing.link-fail", link_action));
   auto g2 = make_unique<testing_compile_link_base_generator>(e,
                                                              "testing.link-fail",
                                                              std::move(link_generator_creator(a2)),
                                                              types::TESTING_LINK_FAIL,
                                                              true);
   e.generators().insert(std::move(g2));
}

}
