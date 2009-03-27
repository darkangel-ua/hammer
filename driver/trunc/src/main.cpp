#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_set.hpp>

#include <iostream>
#include <hammer/core/engine.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include <hammer/core/copy_generator.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/builder.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/types.h>

#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/generic_batcher.h>

#include "user_config_location.h"

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
                         only_up_to_date_check_(false),
                         disable_batcher_(false),
                         hammer_output_dir_(".hammer"),
                         debug_level_(0)
      {}

      vector<string> build_request_options_;
      bool generate_projects_localy_;
      bool only_up_to_date_check_;
      bool disable_batcher_;
      std::string hammer_output_dir_;
      std::string hammer_install_dir_;
      int debug_level_;
      std::string just_one_source_;
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
         ("debug,d", po::value<int>(&opts.debug_level_), "specify verbosity level")
         ("disable-batcher", "do not build many sources at once")
         ("just-one-source,s", po::value<string>(&opts.just_one_source_), "build unconditionally specified source");

      return desc;
   }

   po::options_description options_for_work()
   {
      po::options_description desc(options_for_help());
      desc.add_options()("build-request", po::value<vector<string> >());
      build_request_options.add("build-request", -1);
      return desc;
   }

   static feature* try_resolve_implicit_feature(feature_registry& fr, feature_def& fd, const std::string& value)
   {
      string::size_type p = value.find('-');
      if (p != string::npos)
      {
         string main_feature(value.substr(0, p));
         string subfeature(value.substr(p + 1, value.size() - p - 1));
         if (!fd.is_legal_value(main_feature))
            return NULL;
         if (!fd.find_subfeature_for_value(subfeature))
            return NULL;
      }
      else
      {
         if (!fd.is_legal_value(value))
            return NULL;
      }

      return fr.create_feature(fd.name(), value);
   }
   
   static feature* try_resolve_implicit_feature(feature_registry& fr, const std::string& value)
   {
      feature* result = try_resolve_implicit_feature(fr, fr.get_def("toolset"), value);
      if (result != NULL)
         return result;

      return try_resolve_implicit_feature(fr, fr.get_def("variant"), value);
   }

   void resolve_arguments(vector<string>& targets, feature_set* build_request, 
                          feature_registry& fr, const vector<string>& build_request_options)
   {
      typedef vector<string>::const_iterator iter;
      for(iter i = build_request_options.begin(), last = build_request_options.end(); i != last; ++i)
      {
         string::size_type p = i->find('=');
         if (p == string::npos)
         {
            feature* posible_implicit_feature = try_resolve_implicit_feature(fr, *i);
            if (posible_implicit_feature != NULL)
               build_request->join(posible_implicit_feature);
            else
               targets.push_back(*i);
         }
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
      feature_set* usage_requirements = project.get_engine()->feature_registry().make_set();
      for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         if (is_looks_like_project(*i))
         {
            string target_path, target_name;
            
            split_target_path(target_path, target_name, *i);
            pstring p_target_name(project.get_engine()->pstring_pool(), target_name);

            const hammer::engine::loaded_projects_t& p = project.get_engine()->load_project(target_path, project);
            typedef hammer::project::selected_targets_t selected_targets_t;
            selected_targets_t st = target_name.empty() ? p.select_best_alternative(build_request) :
                                                          selected_targets_t(1, p.select_best_alternative(p_target_name, build_request));
            feature_set* usage_requirements = project.get_engine()->feature_registry().make_set();
            for(selected_targets_t::const_iterator t = st.begin(), t_last = st.end(); t != t_last; ++t)
            {
               t->target_->instantiate(NULL, build_request, &result, usage_requirements);
               usage_requirements->clear();
            }
         }
         else
         {
            pstring name(project.get_engine()->pstring_pool(), *i);
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
             *(**i).products_[0]->get_project() != project)
            i = nodes.erase(i);
         else
            ++i;
      }
   }

   typedef boost::unordered_set<const build_node*> visited_nodes_t;
   typedef boost::unordered_set<const meta_target*> top_targets_t;
   
   // first is signaled that we found source, second - found node to rebuild
   pair<bool, bool> find_node_for_source_name(nodes_t& result,
                                              visited_nodes_t& visited_nodes, 
                                              const boost::intrusive_ptr<build_node>& node,
                                              const pstring& source_name,
                                              const top_targets_t& top_targets)
   {
      if (top_targets.find(node->products_owner().get_meta_target()) == top_targets.end())
         return make_pair(false, false);

      if (visited_nodes.find(node.get()) != visited_nodes.end())
         return make_pair(false, false);

      if (node->sources_.empty() && node->products_.size() == 1)
      {
         if (node->products_.front()->name() == source_name)
            return make_pair(true, false);
         else
            return make_pair(false, false);
      }

      visited_nodes.insert(node.get());

      for(nodes_t::const_iterator i = node->down_.begin(), last = node->down_.end(); i != last; ++i)
      {
         pair<bool, bool> r = find_node_for_source_name(result, visited_nodes, *i, source_name, top_targets);

         if (r.first && r.second)
         {
            visited_nodes.erase(node.get());
            return r;
         }

         if (r.first)
            if (node->is_composite())
            {
               result.push_back(*i);
               visited_nodes.erase(node.get());
               return make_pair(true, true);
            }
            else
            {
               visited_nodes.erase(node.get());
               return r;
            }
      }

      visited_nodes.erase(node.get());
      return make_pair(false, false);
   }

   nodes_t find_nodes_for_source_name(const nodes_t& nodes, const pstring& source_name)
   {
      typedef boost::unordered_set<const meta_target*> top_targets_t;
      top_targets_t top_targets;
      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         top_targets.insert((**i).products_owner().get_meta_target());

      visited_nodes_t visited_nodes;
      nodes_t result;
      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         find_node_for_source_name(result, visited_nodes, *i, source_name, top_targets);
      
      return result;
   }

   void run_build(nodes_t& nodes, 
                  engine& e, 
                  hammer_options opts)
   {
      build_environment_impl build_environment(fs::current_path());
      
      if (opts.just_one_source_.empty())
      {
         actuality_checker checker(e, build_environment);
         cout << "...checking targets for update... ";
         size_t target_to_update_count = checker.check(nodes);
         cout << "Done.\n";

         if (opts.only_up_to_date_check_)
            return;

         if (target_to_update_count == 0)
         {
            cout << "...nothing to update...\n";
            return;
         }

         if (!opts.disable_batcher_)
         {
            cout << "...running batcher... ";
            generic_batcher::process(nodes);
            cout << "Done.\n";
         }

         cout << "...updating " << target_to_update_count << " targets...\n";
         builder builder(build_environment);
         builder.build(nodes);
         cout << "...updated " << target_to_update_count << " targets...\n";
      }
      else
      {
         cout << "...updating source '" << opts.just_one_source_ << "'...\n";
         builder builder(build_environment, true);
         nodes_t source_nodes = find_nodes_for_source_name(nodes, pstring(e.pstring_pool(), opts.just_one_source_));
         builder.build(source_nodes);
         cout << "...updated source '" << opts.just_one_source_ << "'...\n";
      }
   }

//    terminate_function old_terminate_function;
//    void terminate_hander()
//    {
//       cout << "Critical error - terminate handler was invoked\n";
//       old_terminate_function();
//    }

   void use_toolset_rule(project*, engine& e, pstring& toolset_name, pstring& toolset_version, pstring* toolset_home_)
   {
      location_t toolset_home;
      if (toolset_home_ != NULL)
         toolset_home = toolset_home_->to_string();

      e.toolset_manager().init_toolset(e, toolset_name.to_string(), toolset_version.to_string(), toolset_home_ == NULL ? NULL : &toolset_home);
   }
}

int main(int argc, char** argv)
{
   try
   {
//      old_terminate_function = set_terminate(terminate_hander);

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

      engine.generators().insert(std::auto_ptr<generator>(new copy_generator(engine)));
      add_testing_generators(engine, engine.generators());

      if (opts.debug_level_ > 0)
         cout << "Done\n";
      
      if (opts.debug_level_ > 0)
         cout << "...Installing scanners... ";

      boost::shared_ptr<scanner> c_scaner(new hammer::c_scanner);
      engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::CPP), c_scaner);
      engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::C), c_scaner);

      if (opts.debug_level_ > 0)
         cout << "Done\n";

      if (opts.debug_level_ > 0)
         cout << "...Registering known toolsets... ";

      engine.toolset_manager().add_toolset(auto_ptr<toolset>(new msvc_toolset));
      engine.toolset_manager().add_toolset(auto_ptr<toolset>(new gcc_toolset));

      if (opts.debug_level_ > 0)
         cout << "Done\n";

      engine.call_resolver().insert("use-toolset", boost::function<void (project*, pstring&, pstring&, pstring*)>(boost::bind(use_toolset_rule, _1, boost::ref(engine), _2, _3, _4)));

      location_t user_config_script = get_user_config_location();
      if (user_config_script.empty() || !exists(user_config_script))
      {
         if (opts.debug_level_ > 0)
            cout << "...user-config.ham not founded...\n";
      }
      else
      {
         if (opts.debug_level_ > 0)
            cout << "...Loading user-config.ham at '" << user_config_script.native_file_string() << "'...";

         engine.load_hammer_script(user_config_script);
         if (opts.debug_level_ > 0)
            cout << "Done\n";
      }

      if (vm.count("generate-projects-locally"))
         opts.generate_projects_localy_ = true;
      
      if (vm.count("up-to-date-check"))
         opts.only_up_to_date_check_ = true;

      if (vm.count("disable-batcher"))
         opts.disable_batcher_ = true;

      if (vm.count("build-request"))
         resolve_arguments(targets, build_request, engine.feature_registry(), vm["build-request"].as<vector<string> >());

      if (build_request->find("toolset") == build_request->end())
         build_request->join("toolset", "msvc");
      if (build_request->find("variant") == build_request->end())
         build_request->join("variant", "debug");

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

//      remove_propagated_targets(nodes, project_to_build);

      if (vm.count("generate-msvc-8.0-solution"))
         generate_msvc80_solution(nodes, project_to_build);
      else
         run_build(nodes, engine, opts);

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
