#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <signal.h>

#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include <hammer/core/copy_generator.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/obj_generator.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/builder.h>
#include <hammer/core/cleaner.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/copy_target.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/generic_batcher.h>
#include <hammer/core/collect_nodes.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/warehouse.h>

#include "user_config_location.h"
#include "dump_targets_to_update.h"

using namespace std;
using namespace hammer;
using namespace hammer::project_generators;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef vector<boost::intrusive_ptr<build_node> > nodes_t;

namespace
{
   // Ctrl-C flag
   volatile bool interrupt_flag = false;

   // Reaction in Ctrl-C
   volatile bool terminate_immediately = true;

   void already_interrupted_ctrl_handler(int sig)
   {
      signal(SIGINT, already_interrupted_ctrl_handler);
   }

   void ctrl_handler(int sig)
   {
      signal(SIGINT, already_interrupted_ctrl_handler);
      interrupt_flag = true;
      printf("...interrupting...\n");

      if (terminate_immediately)
         terminate();
   }

   unsigned get_number_of_processors()
   {
      return boost::thread::hardware_concurrency();
   }

   struct hammer_options
   {
      vector<string> build_request_options_;
      bool generate_projects_localy_ = false;
      bool only_up_to_date_check_ = false;
      bool disable_batcher_ = false;
      bool clean_all_ = false;
      bool dump_targets_to_update_ = false;
      std::string hammer_output_dir_ = ".hammer";
      int debug_level_ = 0;
      std::string just_one_source_;
      std::string just_one_source_project_path_;
      unsigned worker_count_ = get_number_of_processors();
      bool copy_dependencies_ = false;
      bool write_build_graph_ = false;
      bool update_warehouse_ = false;
      bool add_to_packages_ = false;
      std::string path_to_packages_;
      bool update_all_warehouse_packages_ = false;
      bool release_package_ = false;
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
         ("up-to-date-check,c", po::bool_switch(&opts.only_up_to_date_check_), "instantiate/materialize + generate targets + up to date check")
         ("dump-targets-to-update", po::bool_switch(&opts.dump_targets_to_update_), "dump tree of target to update for debuginf purposes")
         ("clean-all", po::bool_switch(&opts.clean_all_), "clean all targets recursively")
         ("generate-msvc-8.0-solution,p", "generate msvc-8.0 solution+projects")
         ("generate-projects-locally,l", po::bool_switch(&opts.generate_projects_localy_), "when generating build script makes them in one place")
         ("hammer-out", po::value<std::string>(&opts.hammer_output_dir_), "specify where hammer will place all its generated output")
         ("debug,d", po::value<int>(&opts.debug_level_), "specify verbosity level")
         ("disable-batcher", po::bool_switch(&opts.disable_batcher_), "do not build many sources at once")
         ("just-one-source,s", po::value<string>(&opts.just_one_source_), "build unconditionally specified source")
         ("just-one-source-project-path", po::value<string>(&opts.just_one_source_project_path_), "path to project where source reside")
         ("jobs,j", po::value<unsigned>(&opts.worker_count_), "concurrency level")
         ("copy-dependencies", po::bool_switch(&opts.copy_dependencies_), "copy shared modules to output dir when building excecutable")
         ("write-build-graph", po::bool_switch(&opts.write_build_graph_), "don't build, just write graphviz build-graph.dot for building process")
         ("update-warehouse", po::bool_switch(&opts.update_warehouse_), "update warehouse package database")
         ("update-all-warehouse-packages", po::bool_switch(&opts.update_all_warehouse_packages_), "update all warehouse packages that has been changed on the server")
         ("add-to-packages", "add current project into packages database")
         ("path-to-packages", po::value<std::string>(&opts.path_to_packages_), "path to packages database")
         ("release-package", po::bool_switch(&opts.release_package_), "add (release) package current to configured warehouse")
         ;

      return desc;
   }

   po::options_description options_for_work()
   {
      po::options_description desc(options_for_help());
      desc.add_options()("build-request", po::value<vector<string> >());
      build_request_options.add("build-request", -1);
      return desc;
   }

   feature*
   try_resolve_implicit_feature(feature_registry& fr,
                                feature_def& fd,
                                const std::string& value)
   {
      string::size_type p = value.find('-');
      if (p != string::npos)
      {
         string main_feature(value.substr(0, p));
         string subfeature(value.substr(p + 1, value.size() - p - 1));
         if (!fd.is_legal_value(main_feature))
            return NULL;
         if (!fd.find_subfeature_for_value(main_feature, subfeature))
            return NULL;
      }
      else
      {
         if (!fd.is_legal_value(value))
            return NULL;
      }

      return fr.create_feature(fd.name(), value);
   }

   feature*
   try_resolve_implicit_feature(feature_registry& fr,
                                const std::string& value)
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

      std::sort(targets.begin(), targets.end());
      targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
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

   bool project_has_multiple_targets(const hammer::project& p, const string& name)
   {
      unsigned count = 0;
      for(hammer::project::targets_t::const_iterator i= p.targets().begin(), last = p.targets().end(); i != last; ++i)
         if (i->first == name)
            ++count;

      return count > 1;
   }

   vector<basic_target*>
   instantiate_targets(const vector<string>& targets,
                       const hammer::project& project,
                       const feature_set& build_request)
   {
      typedef hammer::project::selected_targets_t selected_targets_t;

      vector<basic_target*> result;
      feature_set* usage_requirements = project.get_engine()->feature_registry().make_set();
      const feature_set* build_request_with_defs = project.get_engine()->feature_registry().add_defaults(build_request.clone());
      for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         if (is_looks_like_project(*i))
         {
            string target_path, target_name;

            split_target_path(target_path, target_name, *i);
            pstring p_target_name(project.get_engine()->pstring_pool(), target_name);

            const hammer::engine::loaded_projects_t& p = project.get_engine()->load_project(target_path, project);
            selected_targets_t st = target_name.empty() ? p.select_best_alternative(*build_request_with_defs) :
                                                          selected_targets_t(1, p.select_best_alternative(p_target_name, *build_request_with_defs));
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
            if (project_has_multiple_targets(project, *i))
            {
               hammer::project::selected_target target = project.select_best_alternative(name, *build_request_with_defs);
               target.target_->instantiate(nullptr, build_request, &result, usage_requirements);
            } else if (const basic_meta_target* bt = project.find_target(name))
               bt->instantiate(nullptr, build_request, &result, usage_requirements);
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

   void find_top_source_project_nodes(const build_node& node,
                                      nodes_t& result,
                                      visited_nodes_t& visited_nodes,
                                      const project* source_project)
   {
      if (visited_nodes.find(&node) != visited_nodes.end())
         return;

      visited_nodes.insert(&node);

      for(nodes_t::const_iterator i = node.down_.begin(), last = node.down_.end(); i != last; ++i)
         if ((*i)->products_owner().get_project() == source_project)
         {
            if (visited_nodes.find(i->get()) == visited_nodes.end())
               result.push_back(*i);
         }
         else
            find_top_source_project_nodes(**i, result, visited_nodes, source_project);
   }

   nodes_t find_top_source_project_nodes(const nodes_t& nodes, const project* source_project)
   {
      visited_nodes_t visited_nodes;
      nodes_t result;

      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         if ((*i)->products_owner().get_project() == source_project)
         {
            if (visited_nodes.find(i->get()) == visited_nodes.end())
               result.push_back(*i);
         }
         else
            find_top_source_project_nodes(**i, result, visited_nodes, source_project);

      return result;
   }

   bool find_node_for_source_name(build_node_ptr& result,
                                  visited_nodes_t& visited_nodes,
                                  const boost::intrusive_ptr<build_node>& node,
                                  const pstring& source_name,
                                  const project* source_project)
  {
      if (visited_nodes.find(node.get()) != visited_nodes.end())
         return false;

      visited_nodes.insert(node.get());

      if (node->products_owner().get_project() == source_project)
         if (node->sources_.empty() && node->products_.size() == 1)
         {
            // founded some source
            if (source_name.is_suffix_of(node->products_.front()->name()))
               return true;
            else
               return false;
         }

      for(nodes_t::const_iterator i = node->down_.begin(), last = node->down_.end(); i != last; ++i)
         if (find_node_for_source_name(result, visited_nodes, *i, source_name, source_project))
         {
            if (!result)
            {
               // this node is source node
               result = *i;
            }

            return true;
         }

      return false;
   }

   build_node_ptr find_nodes_for_source_name(const nodes_t& nodes,
                                             const pstring& source_name,
                                             const project* source_project)
   {
      visited_nodes_t visited_nodes;
      build_node_ptr result;
      for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         if (find_node_for_source_name(result, visited_nodes, *i, source_name, source_project))
            break;

      return result;
   }

   void mark_nodes_to_update(nodes_t& nodes)
   {
      for(build_node_ptr& n : nodes)
         n->up_to_date(boost::tribool::false_value);
   }

   void collect_nodes_for_one_source(visited_nodes_t& visited_nodes,
                                     nodes_t& nodes_to_build,
                                     nodes_t& path,
                                     build_node& node,
                                     const build_node& source_node)
   {
      if (visited_nodes.find(&node) != visited_nodes.end())
         return;

      visited_nodes.insert(&node);
      path.push_back(&node);

      for(build_node_ptr& n : node.down_)
      {
         if (n.get() == &source_node && !path.empty())
         {
            // add path nodes except head
            nodes_t::iterator i = path.begin() + 1;
            for(; i != path.end(); ++i)
               nodes_to_build.push_back(*i);
         }
         else
            collect_nodes_for_one_source(visited_nodes, nodes_to_build, path, *n, source_node);
      }

      path.pop_back();
   }

   void run_build(nodes_t& nodes,
                  engine& e,
                  hammer_options opts)
   {
      terminate_immediately = false;
      signal(SIGINT, ctrl_handler);

      build_environment_impl build_environment(fs::current_path(), opts.debug_level_ >= 3);

      if (opts.just_one_source_.empty())
      {
         actuality_checker checker(e, build_environment);
         cout << "...checking targets for update... " << flush;
         size_t target_to_update_count = checker.check(nodes);
         cout << "Done.\n";

         if (opts.dump_targets_to_update_)
         {
            ofstream f("targets-to-update.txt", std::ios_base::trunc);
            dump_targets_to_update(f, nodes, build_environment);
         }

         if (opts.only_up_to_date_check_)
            return;

         if (target_to_update_count == 0)
         {
            cout << "...nothing to update...\n";
            return;
         }

         if (!opts.disable_batcher_)
         {
            cout << "...running batcher... " << flush;
            generic_batcher::process(nodes, opts.worker_count_);
            cout << "Done.\n";
         }

         cout << "...updating " << target_to_update_count << " targets...\n";
         builder builder(build_environment, interrupt_flag, opts.worker_count_, false);
         if (opts.write_build_graph_)
         {
            ofstream f("build-graph.dot", std::ios_base::trunc);
            builder.generate_graphviz(f, nodes);
            return;
         }

         builder::result build_result = builder.build(nodes);
         cout << "...updated " << build_result.updated_targets_ << " targets...\n";

         if (build_result.failed_to_build_targets_)
            cout << "...failed updating " << build_result.failed_to_build_targets_ << " targets...\n";

         if (build_result.skipped_targets_)
            cout << "...skipped " << build_result.skipped_targets_ << " targets...\n";
      }
      else
      {
//         cout << "...updating source '" << opts.just_one_source_ << "'..." << endl;
         builder builder(build_environment, interrupt_flag, opts.worker_count_, false);

         const project* project_for_source = NULL;
         // FIXME: we must resolve symlinks because under linux fs::current_path give as fully resolved path without any symlinks.
         // That is really wierd I think, but can't find any other solution.
         if (!opts.just_one_source_project_path_.empty())
            project_for_source = &e.load_project(resolve_symlinks(opts.just_one_source_project_path_));

         build_node_ptr source_node = find_nodes_for_source_name(nodes, pstring(e.pstring_pool(), opts.just_one_source_), project_for_source);
         if (!source_node)
            throw std::runtime_error("Source file not founded");

         nodes_t source_project_nodes = find_top_source_project_nodes(nodes, project_for_source);

         // collect all nodes prior to main source project nodes
         visited_nodes_t visited_nodes;
         nodes_t nodes_to_build;
         nodes_t path;
         for(nodes_t::iterator i = source_project_nodes.begin(), last = source_project_nodes.end(); i!= last; ++i)
            collect_nodes_for_one_source(visited_nodes, nodes_to_build, path, **i, *source_node);

         // this source should be rebuilt
         nodes_to_build.push_back(source_node);
         mark_nodes_to_update(nodes_to_build);
/*
         source_node->timestamp(boost::date_time::pos_infin);
         actuality_checker checker(e, build_environment);
         cout << "...checking targets for update... " << flush;
         size_t target_to_update_count = checker.check(nodes_to_build);
         cout << "Done." << endl;

         if (opts.dump_targets_to_update_)
         {
            ofstream f("targets-to-update.txt", std::ios_base::trunc);
            dump_targets_to_update(f, nodes_to_build, build_environment);
         }

         cout << "...building source '" << opts.just_one_source_ << "'..."<< endl;
         builder::result build_result = builder.build(nodes_to_build, project_for_source);
         cout << "...source '" << opts.just_one_source_ << "' builded."<< endl;
*/
         // remove dups
         std::sort(nodes_to_build.begin(), nodes_to_build.end());
         nodes_to_build.erase(std::unique(nodes_to_build.begin(), nodes_to_build.end()), nodes_to_build.end());

         cout << "...building source '" << opts.just_one_source_ << "'..." << endl;
         builder.build(nodes_to_build, project_for_source);
         cout << "...source '" << opts.just_one_source_ << "' builded."<< endl;
      }
   }

   void use_toolset_rule(project*, engine& e, pstring& toolset_name, pstring& toolset_version, pstring* toolset_home_)
   {
      location_t toolset_home;
      if (toolset_home_ != NULL)
         toolset_home = toolset_home_->to_string();

      e.toolset_manager().init_toolset(e, toolset_name.to_string(), toolset_version.to_string(), toolset_home_ == NULL ? NULL : &toolset_home);
   }

   bool has_configured_toolsets(const hammer::engine& e)
   {
      const feature_registry& fs = e.feature_registry();
      const feature_def* toolset_def = fs.find_def("toolset");
      if (!toolset_def)
         throw std::runtime_error("Internal error: Feature 'toolset' not registered");

      return !toolset_def->legal_values().empty();
   }

   void autoconfigure_toolsets(hammer::engine& e)
   {
      e.toolset_manager().autoconfigure(e);
   }

   void do_clean_all(nodes_t& nodes,
                     engine& e)
   {
      cout << "...cleaning...";
      build_environment_impl build_environment(fs::current_path());
      cleaner cleaner(e, build_environment);
      cleaner::result r = cleaner.clean_all(nodes);
      cout << "Done.\n"
           << r.cleaned_target_count_ << " targets was cleaned.\n";
   }

   class dep_copy_target : public file_target
   {
      public:
         dep_copy_target(const location_t& destination,
                         const main_target* mt, const pstring& name,
                         const target_type* t, const feature_set* f)
            : file_target(mt, name, t, f), destination_(destination)
         {}

         virtual const location_t& location() const
         {
            return destination_;
         }

      private:
         location_t destination_;
   };

   void add_copy_dependencies_nodes(nodes_t& nodes, engine& e)
   {
      // collect all executables
      build_node::sources_t executables;
      {
         std::set<const build_node*> visited_nodes;
         std::vector<const target_type*> types_to_collect;
         types_to_collect.push_back(&e.get_type_registry().get(types::EXE));
         collect_nodes(executables, visited_nodes, nodes, types_to_collect, /*recursive=*/false);
      }

      if (executables.empty())
         return;

      if (executables.size() != 1)
         throw std::runtime_error("Dependency copying working only with one exe target");

      // collect all shared lib that needed for executables
      build_node::sources_t shared_libs;
      {
         std::set<const build_node*> visited_nodes;
         std::vector<const target_type*> types_to_collect;
         types_to_collect.push_back(&e.get_type_registry().get(types::SHARED_LIB));
         collect_nodes(shared_libs, visited_nodes, nodes, types_to_collect, /*recursive=*/true);
      }

      const generator& copy_generator = *e.generators()
                                          .find_viable_generators(e.get_type_registry().get(types::COPIED),
                                                                  true, *e.feature_registry().make_set()).at(0).first;

      nodes_t copy_nodes;
      for(build_node::sources_t::const_iterator i = shared_libs.begin(), last = shared_libs.end(); i != last; ++i)
      {
         boost::intrusive_ptr<build_node> new_node(new build_node(executables[0].source_node_->products_owner(), false));
         new_node->targeting_type_ = &e.get_type_registry().get(types::COPIED);
         new_node->action(copy_generator.action());
         new_node->sources_.push_back(*i);
         new_node->down_.push_back(i->source_node_);

         dep_copy_target* new_target = new dep_copy_target(executables[0].source_target_->location(),
                                                           &new_node->products_owner(),
                                                           i->source_target_->name(),
                                                           new_node->targeting_type_,
                                                           &executables[0].source_target_->properties());
         new_node->products_.push_back(new_target);

         copy_nodes.push_back(new_node);
      }

      nodes.insert(nodes.end(), copy_nodes.begin(), copy_nodes.end());
   }

   // http://stackoverflow.com/questions/3758606/how-to-convert-byte-size-into-human-readable-format-in-java
   string human_readable_byte_count(const long long bytes,
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

   class warehouse_dl_notifier : public iwarehouse_download_and_install
   {
      public:
         warehouse_dl_notifier(const unsigned max_package_name_lenght) : max_package_name_lenght_(max_package_name_lenght) {}

         void on_download_begin(const warehouse::package_info& package) override
         {
            cout << "Downloading " << setw(max_package_name_lenght_) << left << (package.name_ + " (" + package.version_ +")")
                 << setw(10) << right << human_readable_byte_count(package.package_file_size_) << flush;
         }

         void on_download_end(const warehouse::package_info& package) override
         {
            cout << " Done" << endl;
         }

         void on_install_begin(const warehouse::package_info& package) override
         {
            cout << "Installing  " << setw(max_package_name_lenght_) << left << (package.name_ + " (" + package.version_ +")")
                 << setw(10) << ' ' << flush;
         }

         void on_install_end(const warehouse::package_info& package) override
         {
            cout << " Done" << endl;
         }

      private:
         const unsigned max_package_name_lenght_;
   };
}


int main(int argc, char** argv) {
   try {
      po::options_description desc(options_for_work());
      po::variables_map vm;
      po::parsed_options options = po::command_line_parser(argc, argv).options(desc).positional(build_request_options).run();
      po::store(options, vm);
      po::notify(vm);

      if (vm.count("help")) {
         cout << "Usage: hammer.exe <options> <targets> <features>\n" << options_for_help();
         return 0;
      }

      // fix concurrency level if user is dumb
      if (opts.worker_count_ == 0)
         opts.worker_count_ = 1;

      if (vm.count("add-to-packages")) {
         opts.add_to_packages_ = true;
         if (!vm.count("path-to-packages"))
            throw std::runtime_error("You should specify path-to-packages parameter");
      }

      while(true) {
         hammer::engine engine;
         install_warehouse_rules(engine.call_resolver(), engine);

         types::register_standart_types(engine.get_type_registry(), engine.feature_registry());

         if (opts.debug_level_ > 0)
            cout << "Done" << endl;

         if (opts.debug_level_ > 0)
            cout << "...Installing generators... " << flush;

         engine.generators().insert(std::auto_ptr<generator>(new copy_generator(engine)));
         engine.generators().insert(std::auto_ptr<generator>(new obj_generator(engine)));
         add_testing_generators(engine, engine.generators());
         add_header_lib_generator(engine, engine.generators());
         install_htmpl(engine);

         if (opts.debug_level_ > 0)
            cout << "Done" << endl;

         if (opts.debug_level_ > 0)
            cout << "...Installing scanners... " << flush;

         boost::shared_ptr<scanner> c_scaner(new hammer::c_scanner);
         engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::CPP), c_scaner);
         engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::C), c_scaner);
         engine.scanner_manager().register_scanner(engine.get_type_registry().get(types::RC), c_scaner);

         if (opts.debug_level_ > 0)
            cout << "Done" << endl;

         if (opts.debug_level_ > 0)
            cout << "...Registering known toolsets... " << flush;

         engine.toolset_manager().add_toolset(auto_ptr<toolset>(new msvc_toolset));
         engine.toolset_manager().add_toolset(auto_ptr<toolset>(new gcc_toolset));
         engine.toolset_manager().add_toolset(auto_ptr<toolset>(new qt_toolset));

         if (opts.debug_level_ > 0)
            cout << "Done" << endl;

         engine.call_resolver().insert("use-toolset", boost::function<void (project*, pstring&, pstring&, pstring*)>(boost::bind(use_toolset_rule, _1, boost::ref(engine), _2, _3, _4)));

         location_t user_config_script = get_user_config_location();
         if (user_config_script.empty() || !exists(user_config_script)) {
            if (opts.debug_level_ > 0)
               cout << "...user-config.ham not founded...\n";
         } else {
            if (opts.debug_level_ > 0)
               cout << "...Loading user-config.ham at '" << user_config_script.string<string>() << "'..." << flush;

            engine.load_hammer_script(user_config_script);
            if (opts.debug_level_ > 0)
               cout << "Done" << endl;
         }

         if (opts.update_warehouse_) {
            warehouse::package_infos_t packages_needs_to_be_updated = engine.warehouse().update();
            if (packages_needs_to_be_updated.empty())
               break;

            cout << "These packages needs to be updates:\n";
            for(const auto& package : packages_needs_to_be_updated)
               cout << package.name_ << "(" << package.version_ << ") size: " << package.package_file_size_ << "\n";

            break;
         }

         if (opts.update_all_warehouse_packages_) {
            engine.warehouse().update_all_packages(engine);
            break;
         }

         autoconfigure_toolsets(engine);

         if (!has_configured_toolsets(engine)) {
            cerr << "WARNING!!!WARNING!!!WARNING!!!WARNING!!!\n"
                    "No toolsets are configured and no one toolset founded by auto-configure!\n"
                    "Please, specify some toolset in $(HOME)/user-config.ham to operate properly.\n"
                    "WARNING!!!WARNING!!!WARNING!!!WARNING!!!\n\n" << flush;
         }

         vector<string> targets;
         feature_set* build_request = engine.feature_registry().make_set();
         if (vm.count("build-request"))
            resolve_arguments(targets, build_request, engine.feature_registry(), vm["build-request"].as<vector<string> >());

         // lets handle 'toolset' feature in build request
#if defined(_WIN32)
         const string default_toolset_name = "msvc";
#else
         const string default_toolset_name = "gcc";
#endif
         auto i_toolset_in_build_request = build_request->find("toolset");
         if (i_toolset_in_build_request == build_request->end()) {
            const feature_def& toolset_definition = engine.feature_registry().get_def("toolset");
            if (!toolset_definition.is_legal_value(default_toolset_name))
               throw std::runtime_error("Default toolset is set to '"+ default_toolset_name + "', but either you didn't configure it in user-config.ham or it has failed to autoconfigure");

            const subfeature_def& toolset_version_def = toolset_definition.get_subfeature("version");
            if (toolset_version_def.legal_values(default_toolset_name).size() == 1)
               build_request->join("toolset", (default_toolset_name + "-" + *toolset_version_def.legal_values(default_toolset_name).begin()).c_str());
            else
               throw std::runtime_error("Default toolset is set to '"+ default_toolset_name + "', but has multiple version configured. You should request specific version to use.");
         } else {
            const feature& used_toolset = **i_toolset_in_build_request;
            if (!used_toolset.find_subfeature("version")) {
               const subfeature_def& toolset_version_def = used_toolset.definition().get_subfeature("version");
               if (toolset_version_def.legal_values(used_toolset.value().to_string()).size() > 1)
                  throw std::runtime_error("Toolset is set to '"+ used_toolset.value().to_string() + "', but has multiple version configured. You should request specific version to use.");
               else {
                  const string toolset = used_toolset.value().to_string();
                  build_request->erase_all("toolset");
                  build_request->join("toolset", (toolset + "-" + *toolset_version_def.legal_values(toolset).begin()).c_str());
               }
            }
         }

         if (build_request->find("variant") == build_request->end())
            build_request->join("variant", "debug");

         if (build_request->find("host-os") == build_request->end())
            build_request->join("host-os", engine.feature_registry().get_def("host-os").get_default().c_str());

         if (opts.debug_level_ > 0)
            cout << "...Build request: " << dump_for_hash(*build_request) << endl;

         if (opts.debug_level_ > 0)
            cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

         const project& project_to_build = engine.load_project(fs::current_path());
         if (opts.debug_level_ > 0)
            cout << "Done" << endl;

         if (opts.add_to_packages_) {
            engine.warehouse().add_to_packages(project_to_build, opts.path_to_packages_);
            break;
         }

         if (opts.release_package_) {
            engine.warehouse().add_to_packages(project_to_build);
            break;
         }

         if (targets.empty() || (targets.size() == 1 && targets[0] == "all")) {
            targets.clear();
            add_all_targets(targets, project_to_build);
         }

         if (opts.debug_level_ > 0) {
            cout << "...Targets to " << (opts.clean_all_ ? "clean-all" : "build") << " is: ";
            bool first_pass = true;
            for(vector<string>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
               if (first_pass)
                  first_pass = false;
               else
                  cout << ',';

               cout << *i;
            }
            cout << endl;
         }

         cout << "...instantiating... " << flush;
         vector<basic_target*> instantiated_targets(instantiate_targets(targets, project_to_build, *build_request));
         cout << "Done." << endl;

         if (vm.count("instantiate"))
            return 0;

         cout << "...generating build graph... " << flush;

         nodes_t nodes;
         try {
            nodes = generate_targets(instantiated_targets);
         } catch(const warehouse_unresolved_target_exception& e) {
            // ups - we have some libs to download
            warehouse& wh = engine.warehouse();
            vector<warehouse::package_info> packages = wh.get_unresoved_targets_info(engine, find_all_warehouse_unresolved_targets(instantiated_targets));

            cout << boost::format("\n\nThere are %d unresolved package(s) to download and install:\n\n") % packages.size();

            auto package_with_longest_name =
               max_element(packages.begin(), packages.end(),
                           [](const warehouse::package_info& lhs, const warehouse::package_info& rhs)
               {
                  return lhs.name_.size() + lhs.version_.size() < rhs.name_.size() + rhs.version_.size();
               });
            const unsigned max_package_name_lenght = package_with_longest_name->name_.size() + package_with_longest_name->version_.size() +
                                                     3 /*formating extras*/;

            long long total_bytes_to_download = 0;
            for(vector<warehouse::package_info>::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i) {
               cout << setw(max_package_name_lenght) << left << (i->name_ + " ("+ i->version_ + ")") << " : " << setw(10) << right << human_readable_byte_count(i->package_file_size_) << endl;
               total_bytes_to_download += i->package_file_size_;
            }
            cout << setw(max_package_name_lenght) << left  << "\nTotal size to download    " << "  : " << setw(10) << right << human_readable_byte_count(total_bytes_to_download) << endl;

            cout << "\n\nDownload & Build? [Y/n]: " << flush;
            char c;
            cin >> c;
            if (c != 'Y' && c != 'y') {
               cout << "Build failed\n";
               return -1;
            }

            warehouse_dl_notifier notifier(max_package_name_lenght);
            wh.download_and_install(engine, packages, notifier);
            continue; // restart
         }

         if (opts.copy_dependencies_)
            add_copy_dependencies_nodes(nodes, engine);

         cout << "Done." << endl;

         if (vm.count("generate"))
            return 0;

   //      remove_propagated_targets(nodes, project_to_build);

         if (opts.clean_all_) {
            do_clean_all(nodes, engine);
            return 0;
         }

         if (vm.count("generate-msvc-8.0-solution"))
            generate_msvc80_solution(nodes, project_to_build);
         else
            run_build(nodes, engine, opts);

         cout << flush;
         cerr << flush;

         break;
      }

      return 0;
   } catch (const std::exception& e) {
      cout << "Error: " << e.what() << "\n";
      return -1;
   } catch (...) {
      cout << "Error: Unknown error.\n";
      return -1;
   }
}
