#include <iostream>
#include <fstream>
#include <iomanip>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/warehouse.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/obj_generator.h>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/builder.h>
#include <hammer/core/generic_batcher.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/instantiation_context.h>
#include "user_config_location.h"
#include "build_cmd.h"
#include "build_request.h"

using namespace std;
using namespace hammer;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

struct build_options_t {
   bool only_instantiate_ = false;
   bool only_generate_ = false;
   bool only_up_to_date_check_ = false;
   bool disable_batcher_ = false;
   bool write_build_graph_ = false;
   unsigned worker_count_ = boost::thread::hardware_concurrency();
   vector<string> build_request_;
} build_options;

po::options_description
build_options_description = []() {
   po::options_description desc("Build options");
   desc.add_options()
      ("instantiate,i", po::bool_switch(&build_options.only_instantiate_), "instantiate targets only")
      ("generate,g", po::bool_switch(&build_options.only_generate_), "instantiate + generate & materialize targets")
      ("up-to-date-check,c", po::bool_switch(&build_options.only_up_to_date_check_), "instantiate + generate & materialize targets + up to date check")
      ("disable-batcher", po::bool_switch(&build_options.disable_batcher_), "do not build many sources at once")
      ("jobs,j", po::value<unsigned>(&build_options.worker_count_), "concurrency level")
      ("write-build-graph", po::bool_switch(&build_options.write_build_graph_), "don't build, just write graphviz build-graph.dot for building process")
   ;

   return desc;
}();

bool has_configured_toolsets(const engine& e) {
   const feature_registry& fs = e.feature_registry();
   const feature_def* toolset_def = fs.find_def("toolset");
   assert(toolset_def && "Internal error: Feature 'toolset' not registered");

   return !toolset_def->legal_values().empty();
}

// http://stackoverflow.com/questions/3758606/how-to-convert-byte-size-into-human-readable-format-in-java
string
human_readable_byte_count(const long long bytes,
                          const bool si = true)
{
   const unsigned unit = si ? 1000 : 1024;
   if (bytes < unit)
      return (boost::format("%1 B") % bytes).str();

   const long exp = lrint(log(bytes) / log(unit));
   return (boost::format("%.1f %c%sB")
            % (bytes / pow(unit, exp))
            % ((si ? "kMGTPE" : "KMGTPE")[exp - 1])
            % (si ? "" : "i")).str();
}

class warehouse_dl_notifier : public iwarehouse_download_and_install {
   public:
      warehouse_dl_notifier(const unsigned max_package_name_lenght) : max_package_name_lenght_(max_package_name_lenght) {}

      bool on_download_begin(const std::size_t index,
                             const warehouse::package_info& package) override
      {
         cout << "Downloading " << setw(max_package_name_lenght_) << left << (package.name_ + " (" + package.version_ +")")
              << setw(10) << right << human_readable_byte_count(package.package_file_size_) << flush;

         return true;
      }

      void on_download_end(const std::size_t index,
                           const warehouse::package_info& package) override
      {
         cout << " Done" << endl;
      }

      bool on_install_begin(const std::size_t index,
                            const warehouse::package_info& package) override
      {
         cout << "Installing  " << setw(max_package_name_lenght_) << left << (package.name_ + " (" + package.version_ +")")
              << setw(10) << ' ' << flush;

         return true;
      }

      void on_install_end(const std::size_t index,
                          const warehouse::package_info& package) override
      {
         cout << " Done" << endl;
      }

   private:
      const unsigned max_package_name_lenght_;
};

void build(engine& e,
           build_nodes_t& nodes,
           const unsigned debug_level,
           volatile bool& interrupt_flag )
{
   build_environment_impl build_environment(fs::current_path(), debug_level >= 3);

   actuality_checker checker(e, build_environment);
   cout << "...checking targets for update... " << flush;
   size_t target_to_update_count = checker.check(nodes);
   cout << "Done" << endl;

   if (build_options.write_build_graph_) {
      ofstream f("build-graph.dot", std::ios_base::trunc);
      builder::generate_graphviz(f, nodes);
      return;
   }

   if (build_options.only_up_to_date_check_)
      return;

   if (target_to_update_count == 0)
      cout << "...nothing to update..." << endl;
   else {
      if (!build_options.disable_batcher_) {
         cout << "...running batcher... " << flush;
         generic_batcher::process(nodes, build_options.worker_count_);
         cout << "Done" << endl;
      }

      cout << "...updating " << target_to_update_count << " targets..." << endl;
      builder builder(build_environment, interrupt_flag, build_options.worker_count_, false);
      builder::result build_result = builder.build(nodes);
      cout << "...updated " << build_result.updated_targets_ << " targets..." << endl;

      if (build_result.failed_to_build_targets_)
         cout << "...failed updating " << build_result.failed_to_build_targets_ << " targets..." << endl;

      if (build_result.skipped_targets_)
         cout << "...skipped " << build_result.skipped_targets_ << " targets..." << endl;
   }
}

void parse_options(const vector<string>& args) {
   auto build_options_description_with_build_request = build_options_description;
   build_options_description_with_build_request.add_options()
      ("build-request", po::value<vector<string>>(&build_options.build_request_), "");

   po::positional_options_description build_request_description;
   build_request_description.add("build-request", -1);

   auto parsed_options =
      po::command_line_parser(args).
         options(build_options_description_with_build_request).
         positional(build_request_description).
         run();

   po::variables_map vm;
   po::store(parsed_options, vm);
   po::notify(vm);

   if (build_options.worker_count_ == 0)
      build_options.worker_count_ = 1;
}

}

std::unique_ptr<hammer::engine>
setup_engine(const unsigned debug_level,
             const bool configure_toolsets) {
   std::unique_ptr<hammer::engine> engine_ptr{new engine};
   engine& engine = *engine_ptr;

   install_warehouse_rules(engine);

   types::register_standart_types(engine.get_type_registry(), engine.feature_registry());

   if (debug_level > 0)
      cout << "Done" << endl;

   if (debug_level > 0)
      cout << "...Installing generators... " << flush;

   engine.generators().insert(std::auto_ptr<generator>(new obj_generator(engine)));
   add_testing_generators(engine, engine.generators());
   add_header_lib_generator(engine, engine.generators());
   install_htmpl(engine);

   if (debug_level > 0)
      cout << "Done" << endl;

   if (debug_level > 0)
      cout << "...Installing scanners... " << flush;

   boost::shared_ptr<scanner> c_scaner(new hammer::c_scanner);
   engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::CPP), c_scaner);
   engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::C), c_scaner);
   engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::RC), c_scaner);

   if (debug_level > 0)
      cout << "Done" << endl;

   if (debug_level > 0)
      cout << "...Registering known toolsets... " << flush;

   engine.toolset_manager().add_toolset(engine, auto_ptr<toolset>(new msvc_toolset));
   engine.toolset_manager().add_toolset(engine, auto_ptr<toolset>(new gcc_toolset));
   engine.toolset_manager().add_toolset(engine, auto_ptr<toolset>(new qt_toolset));

   if (debug_level > 0)
      cout << "Done" << endl;

   location_t user_config_script = get_user_config_location();
   if (user_config_script.empty() || !exists(user_config_script)) {
      if (debug_level > 0)
         cout << "...user-config.ham not founded...\n";
   } else {
      if (debug_level > 0)
         cout << "...Loading user-config.ham at '" << user_config_script.string<string>() << "'..." << flush;

      engine.load_hammer_script(user_config_script);
      if (debug_level > 0)
         cout << "Done" << endl;
   }

   if (configure_toolsets) {
      engine.toolset_manager().autoconfigure(engine);

      if (!has_configured_toolsets(engine)) {
         // FIXME: fix user config location
         cerr << "WARNING!!!WARNING!!!WARNING!!!WARNING!!!\n"
                 "No toolsets were configured and no toolset founded by auto-configure!\n"
                 "Please, specify some toolset in $(HOME)/user-config.ham to operate properly.\n"
                 "WARNING!!!WARNING!!!WARNING!!!WARNING!!!\n\n" << flush;
      }
   }

   return engine_ptr;
}

void resolve_meta_targets(build_request& build_request,
                          const project* p) {
   project::selected_targets_t targets;
   if (build_request.target_names_.empty() ||
       (build_request.target_names_.size() == 1 && build_request.target_names_[0] == "all"))
   {
      targets = p->select_best_alternative(*build_request.build_request_);
   } else {
      for (const string& target_name : build_request.target_names_)
         targets.push_back(p->select_best_alternative(target_name, *build_request.build_request_));
   }

   for (const auto& st : targets)
      build_request.targets_.push_back(st.target_);
}

vector<basic_target*>
instantiate(engine& e,
            const std::vector<const basic_meta_target*>& meta_targets,
            const feature_set& build_request) {
   vector<basic_target*> result;
   feature_set* usage_requirements = e.feature_registry().make_set();
   instantiation_context ctx;
   for (auto& mt : meta_targets) {
      mt->instantiate(ctx, nullptr, build_request, &result, usage_requirements);
      usage_requirements->clear();
   }

   return result;
}

boost::optional<hammer::build_nodes_t>
generate(engine& engine,
         const std::vector<basic_target*> targets) {
   while (true) {
      try {
         build_nodes_t result;
         for (basic_target* t : targets) {
            build_nodes_t n = t->generate();
            result.insert(result.end(), n.begin(), n.end());
         }

         return result;
      } catch (const warehouse_unresolved_target_exception& e) {
         // ups - we have some libs to download
         warehouse& wh = *e.warehouse_;
         vector<warehouse::package_info> packages = wh.get_unresoved_targets_info(engine, find_all_warehouse_unresolved_targets(targets));

         cout << boost::format("\n\nThere are %d unresolved package(s) to download and install:\n\n") % packages.size();

         auto package_with_longest_name =
            max_element(packages.begin(), packages.end(),
                        [](const warehouse::package_info& lhs, const warehouse::package_info& rhs) {
               return lhs.name_.size() + lhs.version_.size() < rhs.name_.size() + rhs.version_.size();
            });
         const unsigned formating_extras = 3;
         const unsigned max_package_name_lenght = package_with_longest_name->name_.size() + package_with_longest_name->version_.size() + formating_extras;

         long long total_bytes_to_download = 0;
         for (const warehouse::package_info& pi : packages) {
            cout << setw(max_package_name_lenght) << left << (pi.name_ + " ("+ pi.version_ + ")") << " : " << setw(10) << right << human_readable_byte_count(pi.package_file_size_) << endl;
            total_bytes_to_download += pi.package_file_size_;
         }
         cout << setw(max_package_name_lenght) << left  << "\nTotal size to download    " << "  : " << setw(10) << right << human_readable_byte_count(total_bytes_to_download) << endl;

         cout << "\n\nDownload & Build? [Y/n]: " << flush;
         char c;
         cin >> c;
         if (c != 'Y' && c != 'y')
            return {};

         warehouse_dl_notifier notifier(max_package_name_lenght);
         wh.download_and_install(engine, packages, notifier);
      }
   }
}

int handle_build_cmd(const std::vector<std::string>& args,
                     const unsigned debug_level,
                     volatile bool& interrupt_flag) {
   auto engine = setup_engine(debug_level);

   if (debug_level > 0)
      cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

   const project* project_to_build = has_project_file(fs::current_path()) ? &engine->load_project(fs::current_path()) : nullptr;

   if (debug_level > 0)
      cout << static_cast<const char*>(project_to_build ? "Done" : "Done (Not Founded)") << endl;

   parse_options(args);

   build_request build_request = resolve_build_request(*engine, build_options.build_request_, project_to_build);

   if (debug_level > 0)
      cout << "...Build request: " << dump_for_hash(*build_request.build_request_) << endl;

   resolve_meta_targets(build_request, project_to_build);

   if (debug_level > 0)
      cout << "...Targets to build are: " << boost::join(build_request.target_names_, ", ") << endl;

   cout << "...instantiating... " << flush;
   vector<basic_target*> instantiated_targets = instantiate(*engine, build_request.targets_, *build_request.build_request_);
   cout << "Done" << endl;

   if (build_options.only_instantiate_)
      return 0;

   cout << "...generating build graph... " << flush;
   boost::optional<build_nodes_t> nodes = generate(*engine, instantiated_targets);
   if (!nodes) {
      cout << "Build failed\n";
      return 1;
   }
   cout << "Done" << endl;

   if (build_options.only_generate_)
      return 0;

   build(*engine, *nodes, debug_level, interrupt_flag);

   return 0;
}

void show_build_cmd_help() {
   cout << "usage: hammer build [options] <build request>\n\n"
        << build_options_description
        << flush;
}
