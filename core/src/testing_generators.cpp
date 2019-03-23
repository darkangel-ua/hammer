#include <boost/make_shared.hpp>
#include <boost/make_unique.hpp>
#include <hammer/core/testing_generators.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/testing_run_action.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/shared_lib_dirs_writer.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/testing_run_meta_target.h>
#include <hammer/core/testing_compile_action.h>
#include <hammer/core/testing_compile_generator.h>
#include <hammer/core/testing_compile_fail_action.h>
#include <hammer/core/testing_compile_fail_generator.h>
#include <hammer/core/testing_link_action.h>
#include <hammer/core/testing_link_generator.h>
#include <hammer/core/testing_link_fail_action.h>
#include <hammer/core/testing_link_fail_generator.h>
#include <hammer/core/target_type.h>
#include <boost/variant/get.hpp>

using std::unique_ptr;
using boost::shared_ptr;
using boost::make_shared;
using boost::make_unique;
namespace fs = boost::filesystem;
using std::string;

namespace hammer {
namespace {

struct testing_run_args_writer : public argument_writer {
   testing_run_args_writer(const type_registry& tr) : argument_writer("args"), runner_type_(tr.get(types::TESTING_RUN_PASSED)) {}
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
      types::TESTING_COMPILE_FAIL,
      types::TESTING_COMPILE_SUCCESSFUL,
      types::TESTING_LINK_FAIL,
      types::TESTING_LINK_SUCCESSFUL,
      types::TESTING_OUTPUT,
      types::TESTING_RUN_PASSED,
      types::TESTING_SUITE
   });

   auto products = make_product_types(e, {types::TESTING_SUITE});
   auto g = make_unique<generator>(e, "testing.suite", sources, products, true, build_action_ptr{});
   g->include_composite_generators(true);
   gr.insert(std::move(g));
}

}

void add_testing_generators(engine& e,
                            generator_registry& gr)
{
   auto run_product = make_shared<product_argument_writer>("run_product", e.get_type_registry().get(types::TESTING_RUN_PASSED));
   auto run_output_product = make_shared<product_argument_writer>("run_output_product", e.get_type_registry().get(types::TESTING_OUTPUT));
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
   cmdline += run_product;
   cmdline += run_output_product;
   cmdline += test_executable;
   cmdline += additional_dirs;
   cmdline += args;

   auto action = std::make_shared<testing_run_action>("testing.run", run_product, run_output_product);
   *action += cmdline;

   auto sources = make_consume_types(e, {types::EXE});
   auto products = make_product_types(e, {types::TESTING_OUTPUT, types::TESTING_RUN_PASSED});

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

   auto a1 = std::make_shared<testing_compile_action>(compile_action);
   e.generators().insert(make_unique<testing_compile_generator>(e, move(compile_generator_creator(a1))));

   auto a2 = std::make_shared<testing_compile_fail_action>(e, compile_action);
   e.generators().insert(make_unique<testing_compile_fail_generator>(e, move(compile_generator_creator(a2))));
}

void add_link_generators(engine& e,
                         const build_action_ptr& link_action,
                         std::function<std::unique_ptr<generator>(const build_action_ptr& action)> link_generator_creator)
{
   e.generators().insert(link_generator_creator(link_action));

   auto a1 = std::make_shared<testing_link_action>(link_action);
   e.generators().insert(make_unique<testing_link_generator>(e, move(link_generator_creator(a1))));

   auto a2 = std::make_shared<testing_link_fail_action>(e, link_action);
   e.generators().insert(make_unique<testing_link_fail_generator>(e, move(link_generator_creator(a2))));
}

}
