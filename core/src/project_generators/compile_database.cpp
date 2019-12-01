#include <cassert>
#include <functional>
#include <boost/unordered_set.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/build_action.h>
#include <hammer/core/project_generators/compile_database.h>
#include <hammer/core/build_environment_impl.h>

namespace fs = boost::filesystem;

namespace hammer {
namespace {

class fake_environment : public build_environment {
   public:
      struct commands_t {
         std::vector<std::string> commands_;
         location_t working_dir_;
      };

      fake_environment(const location_t& current_dir)
         : current_dir_(current_dir)
      {}

      const std::vector<commands_t>&
      commands() const {
         return commands_;
      }

      void reset() { commands_.clear(); }

   private:
      location_t current_dir_;
      mutable std::stringstream output_stream_;
      mutable std::vector<commands_t> commands_;

      bool run_shell_commands(std::ostream* /*captured_output_stream*/,
                              std::ostream* /*captured_error_stream*/,
                              const std::vector<std::string>& cmds,
                              const location_t& working_dir) const override {
         commands_.push_back({cmds, working_dir});
         return true;
      }

      boost::asio::io_context& shell_executor() const override {
         assert("must never be called");
         abort();
      }

      const location_t& current_directory() const override { return current_dir_; }
      void create_directories(const location_t& dir_to_create) const override {}
      void remove(const location_t& p) const override  {}
      void remove_file_by_pattern(const location_t& dir, const std::string& pattern) const override {}
      void copy(const location_t& source, const location_t& destination) const override {}
      bool write_tag_file(const std::string& filename, const std::string& content) const override  { return true; }

      std::unique_ptr<std::ostream>
      create_output_file(const char* filename,
                         std::ios_base::openmode mode) const override {
         return std::unique_ptr<std::ostream>(new std::ostringstream);
      }

      location_t
      working_directory(const basic_build_target& t) const override {
         return current_dir_;
      }

      std::ostream& output_stream() const override { return output_stream_; }
      std::ostream& error_stream() const override { return output_stream_; }

      const location_t* cache_directory() const override { return nullptr; }
};

struct collect_ctx {
      using visited_nodes_t = boost::unordered_set<build_node_ptr>;

      const project& project_;
      const target_type& c_type_;
      const target_type& cpp_type_;
      const target_type& obj_type_;
      visited_nodes_t visited_nodes_;
      build_nodes_t collected_nodes_;
};

void
collect_project_nodes(collect_ctx& ctx,
                      const build_node_ptr& node) {
   if (node->products_owner().get_project() != ctx.project_)
      return;

   if (ctx.visited_nodes_.find(node) != ctx.visited_nodes_.end())
      return;

   ctx.visited_nodes_.insert(node);

   for (auto& down_node : node->down_)
      collect_project_nodes(ctx, down_node);

   if (node->products_.size() != 1 || node->sources_.size() != 1)
      return;

   if (!node->products_.front()->type().equal_or_derived_from(ctx.obj_type_) ||
       !(node->sources_.front().source_target_->type().equal_or_derived_from(ctx.cpp_type_) ||
         node->sources_.front().source_target_->type().equal_or_derived_from(ctx.c_type_)))
      return;

   ctx.collected_nodes_.push_back(node);
}

build_nodes_t
collect_project_nodes(const project& p,
                      const build_nodes_t& nodes) {
   collect_ctx ctx {
      p,
      p.get_engine().get_type_registry().get(types::C),
      p.get_engine().get_type_registry().get(types::CPP),
      p.get_engine().get_type_registry().get(types::OBJ),
   };

   for (auto& node : nodes)
      collect_project_nodes(ctx, node);

   return std::move(ctx.collected_nodes_);
}

struct find_command_result {
   std::reference_wrapper<const std::string> cmd_;
   std::reference_wrapper<const location_t> working_dir_;
};

find_command_result
find_command(const std::vector<fake_environment::commands_t>& commands,
             const std::string& filename) {
   for (const auto& cmd : commands) {
      for (const auto& c : cmd.commands_) {
         if (c.find(filename) != std::string::npos) {
            return {std::ref(c), std::ref(cmd.working_dir_)};
         }
      }
   }

   throw std::runtime_error("Failed to find command for filename '" + filename + "'");
}

std::string
json_escape(std::string s) {
   boost::replace_all(s, "\"", "\\\"");
   return s;
}

}

void generate_compile_database(std::ostream& os,
                               const project& p,
                               const build_nodes_t& nodes) {
   fake_environment env(fs::current_path());
   build_nodes_t project_nodes = collect_project_nodes(p, nodes);

   os << "[\n";
      for (auto& node : project_nodes) {
         env.reset();
         node->action()->execute(*node, env);
         auto& source = *node->sources_.front().source_target_;
         auto source_path = source.location() / source.name();
         source_path.normalize();

         auto cmd = find_command(env.commands(), source_path.filename().string());

         os << "  {\n"
               "    \"directory\": \"" << json_escape(cmd.working_dir_.get().string()) << "\",\n"
               "    \"command\": \"" << json_escape(cmd.cmd_.get()) << "\",\n"
               "    \"file\": \"" << json_escape(source_path.string()) << "\"\n"
               "  }\n";
      }
   os << "]";
}

}
