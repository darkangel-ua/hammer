#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include "../../src/engine.h"
#include "../../src/feature_set.h"
#include "../../src/feature_registry.h"
#include "../../src/type_registry.h"
#include "../../src/basic_target.h"
#include "../../src/main_target.h"
#include "../../src/meta_target.h"
#include "../../src/project_generators/msvc/msvc_solution.h"
#include "../../src/msvc_generator.h"
#include "../../src/copy_generator.h"
#include "../../src/testing_generators.h"
#include "../../src/build_environment_impl.h"
#include "../../src/builder.h"
#include "../../src/actuality_checker.h"
#include "../../src/generator_registry.h"

using namespace std;
using namespace hammer;
using namespace hammer::project_generators;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef vector<boost::intrusive_ptr<build_node> > nodes_t;

namespace
{
   struct hammer_options
   {
      hammer_options() : generate_projects_localy_(false), 
                         hammer_output_dir_(".hammer"),
                         debug_level_(0)
      {}

      vector<string> build_request_options_;
      bool generate_projects_localy_;
      std::string hammer_output_dir_;
      std::string hammer_install_dir_;
      int debug_level_;
   };

   po::positional_options_description build_request_options;
   hammer_options opts;

   po::options_description options_for_help()
   {
      po::options_description desc("General options");
      desc.add_options()
         ("help", "produce this help message")
         ("instantiate,i", "instantiate/materialize targets only")
         ("generate,g", "instantiate/materialize + generate targets")
         ("up-to-date-check,c", "instantiate/materialize + generate targets + up to date check")
         ("generate-msvc-8.0-solution,p", "generate msvc-8.0 solution+projects")
         ("generate-projects-locally,l", "when generating build script makes them in one place")
         ("hammer-out", po::value<std::string>(&opts.hammer_output_dir_), "specify where hammer will place all its generated output")
         ("install-dir", po::value<std::string>(&opts.hammer_install_dir_), "specify where hammer was installed")
         ("debug,d", po::value<int>(&opts.debug_level_), "specify verbosity level");

      return desc;
   }

   po::options_description options_for_work()
   {
      po::options_description desc(options_for_help());
      desc.add_options()("build-request", po::value<vector<string> >());
      build_request_options.add("build-request", -1);
      return desc;
   }

   void resolve_arguments(const vector<string>& build_request_options, vector<string>& targets, 
                          feature_set* build_request)
   {
      typedef vector<string>::const_iterator iter;
      for(iter i = build_request_options.begin(), last = build_request_options.end(); i != last; ++i)
      {
         string::size_type p = i->find('=');
         if (p == string::npos)
            targets.push_back(*i);
         else
         {
            string feature_name(i->begin(), i->begin() + p);
            build_request->join(feature_name.c_str(), i->c_str() + p + 1);
         }
      }
   }

   void add_all_targets(vector<string>& targets, const hammer::project& project)
   {
      for(hammer::project::targets_t::const_iterator i = project.targets().begin(), last = project.targets().end(); i != last; ++i)
      {
         if (!i->second->is_explicit())
            targets.push_back(i->first.to_string());
      }
   }

   bool is_looks_like_project(const string& s)
   {
      return s.find('/') != string::npos;
   }

   void split_target_path(string& target_path, 
                          string& target_name, 
                          const string& to_split)
   {
      boost::smatch match;
      boost::regex pattern("(.+?)//(.+)");
      if (boost::regex_match(to_split, match, pattern))
      {
         target_path = match[1];
         target_name = match[2];
      }
      else
         target_path = to_split;
   }

   vector<basic_target*> 
   instantiate_targets(const vector<string>& targets, const hammer::project& project,
                       const feature_set& build_request)
   {
      vector<basic_target*> result;
      feature_set* usage_requirements = project.engine()->feature_registry().make_set();
      for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         if (is_looks_like_project(*i))
         {
            string target_path, target_name;
            
            split_target_path(target_path, target_name, *i);
            pstring p_target_name(project.engine()->pstring_pool(), target_name);

            const hammer::engine::loaded_projects_t& p = project.engine()->load_project(target_path, project);
            typedef hammer::project::selected_targets_t selected_targets_t;
            selected_targets_t st = target_name.empty() ? p.select_best_alternative(build_request) :
                                                          selected_targets_t(1, p.select_best_alternative(p_target_name, build_request));
            feature_set* usage_requirements = project.engine()->feature_registry().make_set();
            for(selected_targets_t::const_iterator t = st.begin(), t_last = st.end(); t != t_last; ++t)
            {
               (**t).instantiate(NULL, build_request, &result, usage_requirements);
               usage_requirements->clear();
            }
         }
         else
         {
            pstring name(project.engine()->pstring_pool(), *i);
            const basic_meta_target* t = project.find_target(name);
            if (t == NULL)
               throw runtime_error((boost::format("Can't find target '%s'.") % *i).str());
            
            t->instantiate(0, build_request, &result, usage_requirements);
         }
      }
      
      return result;
   }

   nodes_t generate_targets(const vector<basic_target*>& targets)
   {
      typedef vector<basic_target*> targets_t;
      nodes_t result;
      for(targets_t::const_iterator i = targets.begin(), last = targets.end(); i!= last; ++i)
      {
         nodes_t n = (**i).generate();
         result.insert(result.end(), n.begin(), n.end());
      }

      return result;
   }

   void generate_msvc80_solution(const nodes_t& nodes, const hammer::project& project_to_build)
   {
      project_generators::msvc_solution solution(project_to_build, 
                                                 project_to_build.location() / opts.hammer_output_dir_, 
                                                 opts.generate_projects_localy_ ? msvc_solution::generation_mode::LOCAL 
                                                                                : msvc_solution::generation_mode::NON_LOCAL);

      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         solution.add_target(*i);

      solution.write();
   }

   void remove_propagated_targets(nodes_t& nodes, const project& project)
   {
      for(nodes_t::iterator i = nodes.begin(); i != nodes.end();)
      {
         if ((**i).products_.empty() ||
             *(**i).products_[0]->mtarget()->meta_target()->project() != project)
            i = nodes.erase(i);
         else
            ++i;
      }
   }

   void run_build(nodes_t& nodes, bool only_up_to_date_check)
   {
      actuality_checker checker;
      cout << "...checking targets for update... ";
      size_t target_to_update_count = checker.check(nodes);
      cout << "Done\n";
 
      if (only_up_to_date_check)
         return;

      if (target_to_update_count == 0)
      {
         cout << "...nothing to update...\n";
         return;
      }

      cout << "...updating " << target_to_update_count << " targets...\n";
      build_environment_impl build_environment(fs::current_path());
      builder builder(build_environment);
      builder.build(nodes);

      cout << "...updated " << target_to_update_count << " targets...\n";
   }

   terminate_function old_terminate_function;
   void terminate_hander()
   {
      cout << "Critical error - terminate handler was invoked\n";
      old_terminate_function();
   }
}

int main(int argc, char** argv)
{
   try
   {
      old_terminate_function = set_terminate(terminate_hander);

      po::options_description desc(options_for_work());
      po::variables_map vm;
      po::parsed_options options = po::command_line_parser(argc, argv).options(desc).positional(build_request_options).run();
      po::store(options, vm);                 
      po::notify(vm);
      hammer::engine engine;
      vector<string> targets;
      feature_set* build_request = engine.feature_registry().make_set();

      if (vm.count("help"))
      {
         cout << "Usage: hammer.exe <options> <targets> <features>\n" << options_for_help();
         return 0;
      }
      
      fs::path startup_script_dir("./");
      if (vm.count("install-dir"))
         startup_script_dir = opts.hammer_install_dir_;
      
      startup_script_dir /= "scripts/startup.ham";
      if (opts.debug_level_ > 0)
         cout << "...Full path to script is '" << startup_script_dir << "'\n";

      if (opts.debug_level_ > 0)
         cout << "...Loading startup script... ";

      engine.load_hammer_script(startup_script_dir);

      if (opts.debug_level_ > 0)
         cout << "Done\n";

      if (opts.debug_level_ > 0)
         cout << "...Installing generators... ";

      add_msvc_generators(engine, engine.generators());
      engine.generators().insert(std::auto_ptr<generator>(new copy_generator(engine)));
      add_testing_generators(engine, engine.generators());

      if (opts.debug_level_ > 0)
         cout << "Done\n";
      
      build_request->join("toolset", "msvc");
      build_request->join("variant", "debug");

      if (vm.count("generate-projects-locally"))
         opts.generate_projects_localy_ = true;

      if (vm.count("build-request"))
         resolve_arguments(vm["build-request"].as<vector<string> >(), targets, build_request);

      if (opts.debug_level_ > 0)
         cout << "...Loading project at '" << fs::current_path() << "'... ";
      
      const project& project_to_build = engine.load_project(fs::current_path());
      if (opts.debug_level_ > 0)
         cout << "Done\n";

      if (targets.empty())
         add_all_targets(targets, project_to_build);

      if (opts.debug_level_ > 0)
      {
         cout << "...Targets to build is: ";
         bool first_pass = true;
         for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
         {
            if (first_pass)
               first_pass = false;
            else
               cout << ',';

            cout << *i;
         }
         cout << "\n";
      }

      cout << "...instantiating... ";
      vector<basic_target*> instantiated_targets(instantiate_targets(targets, project_to_build, *build_request));
      cout << "Done.\n";

      if (vm.count("instantiate"))
         return 0;

      cout << "...generating build graph... ";
      nodes_t nodes(generate_targets(instantiated_targets));
      cout << "Done.\n";
      
      if (vm.count("generate"))
         return 0;

      remove_propagated_targets(nodes, project_to_build);

      if (vm.count("generate-msvc-8.0-solution"))
         generate_msvc80_solution(nodes, project_to_build);
      else
         run_build(nodes, vm.count("up-to-date-check") != 0);

      return 0;
   }
   catch (const std::exception& e)
   {
      cout << "Error: " << e.what() << "\n";
      return -1;
   }
   catch (...)
   {
      cout << "Error: Unexpected exception.\n";
      return -1;
   }
}
