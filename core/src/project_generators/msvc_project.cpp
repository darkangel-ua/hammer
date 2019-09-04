#include "stdafx.h"
#include <iostream>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <hammer/core/project_generators/msvc_project.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/pch_main_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_argument_writer.h>
#include <hammer/core/free_feature_arg_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/pch_argument_writer.h>

using namespace std;

namespace hammer{ namespace project_generators{

namespace
{
   class fake_environment : public build_environment
   {
      public:
         fake_environment(const location_t& project_output_dir)
            : project_output_dir_(project_output_dir)
         {}

         virtual ~fake_environment() {}

         virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const { return true; }
         virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const { return true; }
         virtual bool run_shell_commands(std::ostream& captured_output_stream, const std::vector<std::string>& cmds, const location_t& working_dir) const { return true; }
         virtual bool run_shell_commands(std::ostream& captured_output_stream, std::ostream& captured_error_stream, const std::vector<std::string>& cmds, const location_t& working_dir) const { return true; }
         virtual const location_t& current_directory() const { return project_output_dir_; }
         virtual void create_directories(const location_t& dir_to_create) const {};
         virtual void remove(const location_t& p) const {};
         virtual void remove_file_by_pattern(const location_t& dir, const std::string& pattern) const {};
         virtual void copy(const location_t& source, const location_t& destination) const {};
         virtual bool write_tag_file(const std::string& filename, const std::string& content) const { return true; }
         virtual std::unique_ptr<std::ostream> create_output_file(const char* filename, std::ios_base::openmode mode) const
         {
            return std::unique_ptr<std::ostream>(new ostringstream);
         }

         location_t working_directory(const basic_build_target& t) const override
         {
            return project_output_dir_;
         }
         virtual std::ostream& output_stream() const { return std::cout; }
         virtual std::ostream& error_stream() const { return std::cerr; }

         const location_t* cache_directory() const { return nullptr; }


      private:
         location_t project_output_dir_;
   };
}

static const string configuration_option_format_string(
   "         CharacterSet=\"$(charset)\">\n");

static const string compiller_option_format_string(
   "            Optimization=\"$(optimization)\"\n"
   "            InlineFunctionExpansion=\"$(inlining)\"\n"
   "            AdditionalIncludeDirectories=\"$(includes)\"\n"
   "            DebugInformationFormat=\"$(debug_format)\"\n"
   "            WarningLevel=\"$(warning_level)\"\n"
   "            PreprocessorDefinitions=\"$(defines)\"\n"
   "            ExceptionHandling=\"$(exception_handling)\"\n"
   "            RuntimeLibrary=\"$(runtime)\"\n"
   "            RuntimeTypeInfo=\"$(rtti)\"\n"
   "            AdditionalOptions=\"$(cxxflags)$(cflags)\"\n"
   "            UsePrecompiledHeader=\"$(pch_type)\"\n"
   "            PrecompiledHeaderThrough=\"$(pch_header)\"\n");

static const string linker_option_format_string(
   "            AdditionalDependencies='$(additional_libraries) $(additional_searched_libraries)'\n"
   "            AdditionalLibraryDirectories=\"$(additional_libraries_dirs)\"\n"
   "            GenerateDebugInformation=\"$(debug_info)\"\n"
   "            LinkIncremental=\"$(incremental_linking)\"\n"
   "            SubSystem=\"$(subsystem)\"\n");

static const string post_build_step_format_string("$(non_path_args) $(path_args)\"\n");

msvc_project::msvc_project(engine& e,
                           const location_t& output_dir,
                           const std::string& solution_configuration_name,
                           const boost::uuids::uuid& uid)
   :
    engine_(&e),
    uid_(uid),
    output_dir_(output_dir),
    project_output_dir_(output_dir_),
    solution_configuration_name_(solution_configuration_name),
    searched_lib_(engine_->get_type_registry().get(types::SEARCHED_LIB)),
    obj_type_(engine_->get_type_registry().get(types::OBJ)),
    pch_type_(engine_->get_type_registry().get(types::PCH)),
    testing_run_passed_type_(engine_->get_type_registry().get(types::TESTING_RUN)),
    copy_type_(engine_->get_type_registry().get(types::COPY)),
    exe_type_(engine_->get_type_registry().get(types::EXE)),
    configuration_options_(configuration_option_format_string),
    compiller_options_(compiller_option_format_string),
    linker_options_(linker_option_format_string),
    post_build_step_(post_build_step_format_string)
{
   output_dir_.normalize();
   // configuration options
   std::shared_ptr<fs_argument_writer> charset(new fs_argument_writer("charset", e.feature_registry()));
   charset->add("<character-set>unicode", "1").
            add("<character-set>multi-byte", "2").
            add("<character-set>unknown", "0");
   configuration_options_ += charset;

   // compiller options
   std::shared_ptr<fs_argument_writer> optimization(new fs_argument_writer("optimization", e.feature_registry()));
   optimization->add("<optimization>off", "0").
                 add("<optimization>speed", "2").
                 add("<optimization>space", "1");

   compiller_options_ += optimization;

   std::shared_ptr<fs_argument_writer> inlining(new fs_argument_writer("inlining", e.feature_registry()));
   inlining->add("<inlining>off", "0").
             add("<inlining>on", "1").
             add("<inlining>full", "2");
   compiller_options_ += inlining;

   std::shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry(), "include", string(), string(), ";"));
   compiller_options_ += includes;

   std::shared_ptr<fs_argument_writer> debug_format(new fs_argument_writer("debug_format", e.feature_registry()));
   debug_format->add("<debug-symbols>on/<profiling>off", "4").
                 add("<debug-symbols>on/<profiling>on",  "3").
                 add("<debug-symbols>off",  "0");
   compiller_options_ += debug_format;

   std::shared_ptr<fs_argument_writer> warning_level(new fs_argument_writer("warning_level", e.feature_registry()));
   warning_level->add("<warnings>on", "3").
                  add("<warnings>off", "0").
                  add("<warnings>all", "4");
   compiller_options_ += warning_level;

   std::shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry(), "define", string(), string(), ";"));
   compiller_options_ += defines;

   std::shared_ptr<fs_argument_writer> exception_handling(new fs_argument_writer("exception_handling", e.feature_registry()));
   exception_handling->add("<exception-handling>off", "0").
                       add("<exception-handling>on/<asynch-exceptions>off", "1").
                       add("<asynch-exceptions>on", "2");
   compiller_options_ += exception_handling;

   std::shared_ptr<fs_argument_writer> runtime(new fs_argument_writer("runtime", e.feature_registry()));
   runtime->add("<runtime-link>static/<runtime-debugging>off", "0").
            add("<runtime-link>static/<runtime-debugging>on", "1").
            add("<runtime-link>shared/<runtime-debugging>off", "2").
            add("<runtime-link>shared/<runtime-debugging>on", "3");
   compiller_options_ += runtime;

   std::shared_ptr<fs_argument_writer> rtti(new fs_argument_writer("rtti", e.feature_registry()));
   rtti->add("<rtti>on", "true").
         add("<rtti>off", "false");
   compiller_options_ += rtti;

   std::shared_ptr<free_feature_arg_writer> cxxflags(new free_feature_arg_writer("cxxflags", e.feature_registry(), "cxxflags"));
   compiller_options_ += cxxflags;

   std::shared_ptr<free_feature_arg_writer> cflags(new free_feature_arg_writer("cflags", e.feature_registry(), "cflags"));
   compiller_options_ += cflags;

   std::shared_ptr<pch_argument_writer> pch_header(new pch_argument_writer("pch_header"));
   compiller_options_ += pch_header;

   std::shared_ptr<fs_argument_writer> pch_type(new fs_argument_writer("pch_type", e.feature_registry()));
   pch_type->add("<pch>use", "2").
             add("<pch>create", "1").
             add("<pch>off", "0");
   compiller_options_ += pch_type;

   // linker options
   std::shared_ptr<source_argument_writer> additional_libraries(
       new source_argument_writer("additional_libraries",
                                  e.get_type_registry().get(types::PREBUILT_STATIC_LIB),
                                  true,
                                  source_argument_writer::RELATIVE_TO_WORKING_DIR));
   linker_options_ += additional_libraries;

   std::shared_ptr<source_argument_writer> additional_searched_libraries(
       new source_argument_writer("additional_searched_libraries",
                                  e.get_type_registry().get(types::SEARCHED_STATIC_LIB),
                                  true,
                                  source_argument_writer::WITHOUT_PATH));
   linker_options_ += additional_searched_libraries;

   std::shared_ptr<free_feature_arg_writer> additional_libraries_dirs(
       new free_feature_arg_writer("additional_libraries_dirs",
                                   e.feature_registry(),
                                   "search",
                                   string(), string(), ";"));
   linker_options_ += additional_libraries_dirs;

   std::shared_ptr<fs_argument_writer> link_debug_info(new fs_argument_writer("debug_info", engine_->feature_registry()));
   link_debug_info->add("<debug-symbols>on", "true").
                    add("<debug-symbols>off", "false");
   linker_options_ += link_debug_info;

   std::shared_ptr<fs_argument_writer> link_subsystem(new fs_argument_writer("subsystem", engine_->feature_registry()));
   link_subsystem->add("<user-interface>console", "1").
                   add("<user-interface>gui", "2");
   linker_options_ += link_subsystem;

   std::shared_ptr<fs_argument_writer> incremental_linking(new fs_argument_writer("incremental_linking", engine_->feature_registry()));
   incremental_linking->add("<debug-symbols>on", "2").
                        add("<debug-symbols>off", "1");
   linker_options_ += incremental_linking;

   // Post Build Step section
   std::shared_ptr<free_feature_arg_writer> path_args(
       new free_feature_arg_writer("path_args",
                                   e.feature_registry(),
                                   "testing.input-file"));
   post_build_step_ += path_args;

   std::shared_ptr<free_feature_arg_writer> non_path_args(
       new free_feature_arg_writer("non_path_args",
                                   e.feature_registry(),
                                   "testing.argument"));
   post_build_step_ += non_path_args;
}

static std::string make_variant_name(const main_target& mt)
{
   const feature& f = mt.properties().get("variant");
   return f.value() + '-' + mt.hash_string();
}

void msvc_project::add_variant(boost::intrusive_ptr<const build_node> node)
{
   assert(!node->products_.empty());
   std::unique_ptr<variant> v(new variant);
   variant* naked_variant = v.get();

   // if this testing runner than we actually need exe target that we run in post build step
   if (*node->targeting_type_ == testing_run_passed_type_)
   {
      naked_variant->real_node_ = node;
      node = node->sources_.front().source_node_;
   }
   else
      naked_variant->real_node_ = node;

   const basic_build_target* t = node->products_[0];
   v->properties_ = &t->properties();
   v->node_ = node;
   v->target_ = node->products_[0]->get_main_target();
   v->name_ = make_variant_name(*v->target_);
   v->owner_ = this;
   variants_.push_back(std::move(v));
   if (id_.empty())
   {
      meta_target_ = naked_variant->target_->get_meta_target();
      project_output_dir_ = output_dir() / name();
      project_output_dir_.normalize();
      id_ = project_output_dir().string();
      full_project_name_ = project_output_dir() / (name() + ".vcproj");
      meta_target_relative_to_output_ = relative_path(meta_target_->location(), project_output_dir());
      meta_target_relative_to_output_.normalize();
      if (uid_.is_nil())
      {
         uid_ = boost::uuids::random_generator{}();

         if (exists(full_project_name()))
         {
            string content;
            boost::filesystem::ifstream f(full_project_name());
            copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), back_inserter(content));

            boost::regex pattern("ProjectGUID=\"\\{([^\\}]+)\\}\"");
            boost::smatch match;
            if (boost::regex_search(content, match, pattern))
               uid_ = boost::uuids::string_generator{}(std::string(match[1]));
         }
      }
   }
}

void msvc_project::fill_filters()
{
   filter_t::types_t source_types;
   source_types.push_back(&engine_->get_type_registry().get(types::CPP));
   source_types.push_back(&engine_->get_type_registry().get(types::C));
   files_.push_back(filter_t(source_types, "Source Files", "{4FC737F1-C7A5-4376-A066-2A32D752A2FF}"));
   filter_t::types_t header_types;
   header_types.push_back(&engine_->get_type_registry().get(types::H));
   files_.push_back(filter_t(header_types, "Header Files", "{93995380-89BD-4b04-88EB-625FBE52EBFB}"));

   filter_t::types_t copy_types;
   copy_types.push_back(&copy_type_);
   files_.push_back(filter_t(copy_types, "Files to copy"));

}

const std::string msvc_project::name() const
{
   string version = variants_.front()->target_->version();
   if (version.empty())
      return variants_.front()->target_->get_meta_target()->name();
   else
      return variants_.front()->target_->get_meta_target()->name() + '-' + version;
}

void msvc_project::write_header(ostream& s) const
{
   s << "<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n"
        "<VisualStudioProject\n"
        "   ProjectType=\"Visual C++\"\n"
        "   Version=\"8,00\"\n"
        "   Name=\"" << name() <<"\"\n"
        "   ProjectGUID=\"{" << uid_ << "}\"\n"
        "   RootNamespace=\"" << name() << "\"\n"
        "   Keyword=\"Win32Proj\">\n"
        "       <Platforms>\n"
        "       <Platform Name=\"Win32\"/>\n"
        "   </Platforms>\n"
        "   <ToolFiles/>\n";
}

static void write_bottom(std::ostream& s)
{
   s << "</VisualStudioProject>";
}

configuration_types::value msvc_project::resolve_configuration_type(const variant& v) const
{
   const target_type& exe_type = engine_->get_type_registry().get(types::EXE);
   const target_type& static_lib_type = engine_->get_type_registry().get(types::STATIC_LIB);
   const target_type& shared_lib_type = engine_->get_type_registry().get(types::SHARED_LIB);
   const target_type& header_lib_type = engine_->get_type_registry().get(types::HEADER_LIB);

   if (v.target_->type().equal_or_derived_from(exe_type) ||
       v.target_->type().equal_or_derived_from(testing_run_passed_type_))
   {
      return configuration_types::exe;
   }
   else
      if (v.target_->type().equal_or_derived_from(static_lib_type))
         return configuration_types::static_lib;
      else
         if (v.target_->type().equal_or_derived_from(shared_lib_type))
            return configuration_types::shared_lib;
         else
            if (v.target_->type().equal_or_derived_from(header_lib_type) ||
                v.target_->type().equal_or_derived_from(copy_type_))
            {
               return configuration_types::utility;
            }
            else
               throw std::runtime_error("[msvc_project] Can't resolve configurations type '" + v.target_->type().tag().name() + "'.");
}

static void write_compiler_options(std::ostream& s,
                                   const cmdline_builder& formater,
                                   const build_node& node,
                                   const build_environment& environment)
{
   s << "         <Tool\n"
      "            Name=\"VCCLCompilerTool\"\n";
   formater.write(s, node, environment);
   s << "         />\n";
}

void msvc_project::write_configurations(std::ostream& s) const
{
   s << "   <Configurations>\n";

   for (auto& i : variants_)
   {
      configuration_types::value cfg_type = resolve_configuration_type(*i);
      s << "      <Configuration\n"
           "         Name=\"" << i->name_ << "|Win32\"\n"
           "         OutputDirectory=\"$(SolutionDir)" << solution_configuration_name_ << "\"\n"
           "         IntermediateDirectory=\"$(ConfigurationName)\"\n"
           "         ConfigurationType=\"" << cfg_type << "\"\n";

      fake_environment fe(project_output_dir());
      configuration_options_.write(s, *i->node_, fe);

      write_compiler_options(s, compiller_options_, *i->node_, fe);

      switch(cfg_type)
      {
         case configuration_types::exe:
         case configuration_types::shared_lib:
         {
            s << "         <Tool\n"
                 "            Name=\"VCLinkerTool\"\n";
            linker_options_.write(s, *i->node_, fe);
            s << "         />\n";
         }
         break;
         case configuration_types::static_lib:
         case configuration_types::utility:
         break;
      }

      if (i->real_node_->targeting_type_->equal_or_derived_from(testing_run_passed_type_))
      {
         s << "         <Tool\n"
              "            Name=\"VCPostBuildEventTool\"\n"
           << "            CommandLine=\"$(TargetPath) ";

         post_build_step_.write(s, *i->real_node_, fe);

         s << "         />\n";
      }

      s << "      </Configuration>\n";
   }

   s << "   </Configurations>\n";
}
/* defined but not used
static feature_set* compute_file_conf_properties(const basic_target& target, const msvc_project::variant& v)
{
   feature_set* result = v.target_->get_engine()->feature_registry().make_set();
   for(feature_set::const_iterator i = target.properties().begin(), last = target.properties().end(); i != last; ++i)
   {
      feature_set::const_iterator f = v.properties_->find(**i);
      if (f == v.properties_->end())
         result->join(*i);
   }

   return result;
}
*/
void msvc_project::file_configuration::write(write_context& ctx, const variant& v) const
{
   if (!v.properties_->contains(target_->properties()))
   {
      ctx.output_ << "              <FileConfiguration\n"
                  << "                   Name=\"" << v.name_ << "\">\n";
      write_compiler_options(ctx.output_, ctx.compiller_options_, *node_, ctx.environment_);
      ctx.output_ << "              </FileConfiguration>\n";
   }
}

void msvc_project::file_with_cfgs_t::write(write_context& ctx, const std::string& path_prefix) const
{
   location_t file_name(location_t(path_prefix) / file_name_);
   file_name.normalize();
   ctx.output_ << "         <File\n"
                  "            RelativePath=\"" << file_name.string() << "\">\n";

   for(file_config_t::const_iterator i = file_config.begin(), last = file_config.end(); i != last; ++i)
      if (*i->first->properties_ != i->second.target_->properties() &&
          &i->second.target_->type() != &ctx.h_type_) // discard any differences for H targets. Done for PCH. May be FIXME:
      {
         i->second.write(ctx, *i->first);
      }

   ctx.output_ << "         </File>\n";
}

struct less_target
{
   bool operator ()(const basic_build_target* lhs, const basic_build_target* rhs)
   {
      location_t lhs_id = lhs->location() / lhs->name();
      location_t rhs_id = rhs->location() / rhs->name();
      lhs_id.normalize();
      rhs_id.normalize();

      return lhs_id < rhs_id;
   }
};

void msvc_project::filter_t::write(write_context& ctx, const std::string& path_prefix) const
{
   if (files_.empty())
      return;

   ctx.output_ << "         <Filter\n"
                  "            Name=\"" << name << "\"";
   if (!uid.empty())
      ctx.output_ << "\n            UniqueIdentifier=\"" << uid << "\">\n";
   else
      ctx.output_ << ">\n";

   // FIXME: this trick used only for test to stabilize order of sources in project file
   typedef std::map<const basic_build_target*, boost::reference_wrapper<const file_with_cfgs_t>, less_target> stabilized_t;
   stabilized_t stabilized;

   for(const files_t::value_type& f : files_)
      stabilized.insert(make_pair(f.first, boost::reference_wrapper<const file_with_cfgs_t>(f.second)));

   for(stabilized_t::const_iterator i = stabilized.begin(), last = stabilized.end(); i != last; ++i)
      i->second.get().write(ctx, path_prefix);

   ctx.output_ << "         </Filter>\n";
}

void msvc_project::write_files(write_context& ctx) const
{
   ctx.output_ << "      <Files>\n";
   string path_prefix = meta_target_relative_to_output_.string();
   for(files_t::const_iterator i = files_.begin(), last = files_.end(); i != last; ++i)
      i->write(ctx, path_prefix);

   ctx.output_ << "      </Files>\n";
}

void msvc_project::generate()
{
   if (variants_.empty())
      throw runtime_error("Can't generate empty msvc project");

   fill_filters();
   gether_files();
}

void msvc_project::write() const
{
   if (variants_.empty())
      throw runtime_error("Can't write empty msvc project");

   create_directories(full_project_name_.branch_path());
   boost::filesystem::ofstream f(full_project_name_, std::ios_base::trunc);
   write_header(f);
   write_configurations(f);

   fake_environment environment(project_output_dir());
   write_context ctx(f, engine_->get_type_registry().get(types::H), environment, compiller_options_);
   write_files(ctx);

   write_bottom(f);
}

bool msvc_project::filter_t::accept(const target_type* t) const
{
   for(types_t::const_iterator i = types_.begin(), last = types_.end(); i != last; ++i)
   {
      if (*i == t)
         return true;
   }

   return false;
}

void msvc_project::filter_t::insert(const boost::intrusive_ptr<build_node>& node,
                                    const basic_build_target* t,
                                    const variant& v)
{
   file_with_cfgs_t& fwc = files_[t];
   fwc.file_name_ = t->name();
   file_configuration& fc = fwc.file_config[&v];
   fc.exclude_from_build = false;
   fc.target_ = t;
   fc.node_ = node;
}

void msvc_project::insert_into_files(const boost::intrusive_ptr<build_node>& node,
                                     const basic_build_target* t,
                                     const variant& v)
{
   const target_type* tp = &t->type();
   for(files_t::iterator fi = files_.begin(), flast = files_.end(); fi != flast; ++fi)
   {
      if (fi->accept(tp))
      {
         fi->insert(node, t, v);
         return;
      }
   }
}

void msvc_project::gether_files_impl(const build_node& node, variant& v)
{
   typedef build_node::sources_t::const_iterator iter;
   for(iter mi = node.sources_.begin(), mlast = node.sources_.end(); mi != mlast; ++mi)
   {
      if (mi->source_target_->get_meta_target() == meta_target_ ||
          mi->source_target_->get_main_target()->type().equal_or_derived_from(obj_type_) ||
          mi->source_target_->get_main_target()->type().equal_or_derived_from(pch_type_))
      {
         insert_into_files(mi->source_node_, mi->source_target_, v);
         if (mi->source_node_)
            gether_files_impl(*mi->source_node_, v);
      }
      else
      {
         if (mi->source_target_->get_main_target()->type().equal_or_derived_from(searched_lib_))
         { // this target is searched lib product
            const std::string& file_name = mi->source_target_->name();
            location_t searched_file(mi->source_target_->location().empty()
                                       ? file_name
                                       : relative_path(mi->source_target_->location(), project_output_dir_) / file_name);
            searched_file.normalize();
         }
         else
            dependencies_.push_back(mi->source_target_->get_main_target());
      }
   }
}

void msvc_project::gether_files()
{
   for (auto& i : variants_)
      gether_files_impl(*i->node_, *i);

   std::sort(dependencies_.begin(), dependencies_.end());
   dependencies_.erase(std::unique(dependencies_.begin(), dependencies_.end()), dependencies_.end());
}

bool msvc_project::has_variant(const main_target* v) const
{
   for (auto& i : variants_) {
      if (i->target_ == v)
         return true;
   }

   return false;
}

}}
