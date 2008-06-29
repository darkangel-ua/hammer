#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <iostream>
#include "../../src/engine.h"
#include "../../src/feature_set.h"
#include "../../src/feature_registry.h"
#include "../../src/type_registry.h"
#include "../../src/basic_target.h"
#include "../../src/project_generators/msvc/msvc_solution.h"

using namespace std;
using namespace hammer;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef vector<boost::intrusive_ptr<build_node> > nodes_t;

namespace
{
   struct hammer_options
   {
      vector<string> build_request_options_;
   };

   po::options_description desc("General options");
   po::positional_options_description build_request_options;
   hammer_options opts;

   void init_options()
   {
      desc.add_options()
         ("help", "produce this help message");
      build_request_options.add("build-request", -1);
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
         targets.push_back(i->first.to_string());
   }

   vector<basic_target*> 
   instantiate_targets(const vector<string>& targets, const hammer::project& project,
                       const feature_set& build_request)
   {
      vector<basic_target*> result;
      feature_set* usage_requirements = project.engine()->feature_registry().make_set();
      for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         pstring name(project.engine()->pstring_pool(), *i);
         project.find_target(name)->instantiate(0, build_request, &result, usage_requirements);
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

   void generate_msvc80_solution(const nodes_t& nodes, hammer::engine& engine)
   {
      project_generators::msvc_solution solution(engine);
      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         solution.add_target(*i);

      solution.write();
   }
}

int main(int argc, char** argv)
{
   try
   {
      if (argc == 1)
      {
         cout << "Type 'hammer --help' for usage.\n";
         return 0;
      }
      
      init_options();
      po::variables_map vm;
      po::parsed_options options = po::command_line_parser(argc, argv).options(desc).positional(build_request_options).run();
      po::store(options, vm);                 
      po::notify(vm);
      hammer::engine engine;
      vector<string> targets;
      feature_set* build_request = engine.feature_registry().make_set();

      if (vm.count("help"))
      {
         cout << desc;
         return 0;
      }
      
      if (vm.count("build-request"))
         resolve_arguments(vm["build-request"].as<vector<string> >(), targets, build_request);

      const project& project_to_build = engine.load_project(fs::current_path());
      if (targets.empty())
         add_all_targets(targets, project_to_build);

      vector<basic_target*> instantiated_targets(instantiate_targets(targets, project_to_build, *build_request));
      nodes_t nodes(generate_targets(instantiated_targets));

      generate_msvc80_solution(nodes, engine);

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
