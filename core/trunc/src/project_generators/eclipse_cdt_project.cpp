#include "stdafx.h"
#include <hammer/core/project_generators/eclipse_cdt_project.h>
#include <hammer/core/project_generators/eclipse_cdt_workspace.h>
#include <boost/filesystem/convenience.hpp>
#include <hammer/core/project.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/feature.h>
#include <hammer/core/fs_helpers.h>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

namespace fs = boost::filesystem;

using namespace std;

namespace hammer{ namespace project_generators{

typedef std::map<pstring /*name*/, const basic_meta_target*> meta_targets_t;

eclipse_cdt_project::eclipse_cdt_project(const eclipse_cdt_workspace& workspace,
                                         const hammer::project& project)
   : workspace_(workspace),
     project_(project),
     is_master_project_(false),
     should_copy_dependencies_(false)
{

}

static meta_targets_t 
get_non_explicit_targets(const project& project)
{
   meta_targets_t result;
   for(project::targets_t::const_iterator i = project.targets().begin(), last = project.targets().end(); i != last; ++i)
      if (!i->second->is_explicit())
         result.insert(make_pair(i->second->name(), i->second));

   return result;
}

static void add_link(ostream& os, const location_t& l, const string& name)
{
   os << "\t\t<link>\n"
      "\t\t\t<name>" << name << "</name>\n"
      "\t\t\t<type>2</type>\n"
      "\t\t\t<location>" << l.string() << "</location>\n"
      "\t\t</link>\n";
}

static string escape_for_regex(const string& s)
{
   string result(s);
   boost::replace_all(result, ".", "\\.");
   boost::replace_all(result, ":", "\\:");
   return result;
}

static bool directory_has_files(fs::path path)
{
   for(fs::directory_iterator i = fs::directory_iterator(path), last = fs::directory_iterator(); i != last; ++i)
      if (i->status().type() != fs::directory_file)
         return false;

   return true;
}

void eclipse_cdt_project::construct(const project_main_targets_t& targets)
{
   cdt_id_ = 1;
   // trying to induce project name
   if (project_.name().empty())
   {
      if (project_.targets().size() == 1)
         project_name_ = project_.targets().begin()->first.to_string();
      else
      {
         meta_targets_t meta_targets(get_non_explicit_targets(project_));
         if (meta_targets.size() == 1)
            project_name_ = meta_targets.begin()->second->name().to_string();
         else
            throw std::runtime_error("Can't induce project name for hammer project at '" + project_.location().native_file_string() + "'");
      }
   }
   else
      project_name_ = project_.name().to_string();

   // fill CDT identificators. Really don't know what is it.
   project_id_ = make_cdt_id();
   configuration_id_ = "0\\." + make_cdt_id();
   toolchain_id_ = make_cdt_id();
   platform_id_ = make_cdt_id();
   builder_id_ = make_cdt_id();
   holder_libs_id_ = make_cdt_id();
   assembler_settings_holder_id_ = make_cdt_id();
   assembler_holder_intype_id_ = make_cdt_id();
   cpp_settings_holder_id_ = make_cdt_id();
   cpp_holder_intype_id_ = make_cdt_id();
   c_settings_holder_id_ = make_cdt_id();
   c_holder_intype_id_ = make_cdt_id();

   // trying determinate folder structure and project root
   std::stringstream links_buffer;
   if (!targets.empty() && 
       (**targets.begin()).properties().find("project-root") != (**targets.begin()).properties().end())
   {
      const feature* f = *(**targets.begin()).properties().find("project-root");
      location_t project_root = f->get_path_data().target_->location() / f->value().to_string();
      project_root.normalize();
      add_link(links_buffer, project_root, "root");
   }
   else
   {
	   if (directory_has_files(project_.location().branch_path()))
	   {
			if (project_.location().filename() == "build")
				add_link(links_buffer, project_.location().branch_path(), "root");
			else
				add_link(links_buffer, project_.location(), "root");
	   }
	   else
	   {
		   // directory has no files only directories
		   for(fs::directory_iterator i = fs::directory_iterator(project_.location().branch_path()), last = fs::directory_iterator(); i != last; ++i)
		      if (i->status().type() == fs::directory_file && i->path().filename() != ".svn")
		    	  add_link(links_buffer, project_.location().branch_path(), i->path().filename());
	   }
   }

   links_ = escape_for_regex(links_buffer.str());

   // gather include paths
   typedef boost::unordered_set<const feature*> include_features_t;
   typedef boost::unordered_set<location_t> include_paths_t;
   include_features_t include_features;
   include_paths_t include_paths;
   for(project_main_targets_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
   {
      const feature_set& fs = (**i).properties();
      for(feature_set::const_iterator f = fs.find("include"), f_last = fs.end(); f != f_last; f = fs.find(++f, "include"))
      {
         if (include_features.insert(*f).second)
         {
            location_t include_path((**f).get_path_data().target_->location() / (**f).value().to_string());
            include_path.normalize();
            include_paths.insert(include_path);
         }
      }
   }

   // add include to project file
   ostringstream includes_buffer;
   includes_buffer <<
      "<option id=\"org.eclipse.cdt.build.core.settings.holder.incpaths." + make_cdt_id() + "\"\n"
      "        superClass=\"org.eclipse.cdt.build.core.settings.holder.incpaths\"\n"
      "        valueType=\"includePath\">\n";
   for(include_paths_t::const_iterator i = include_paths.begin(), last = include_paths.end(); i != last; ++i)
   {
      includes_buffer <<   
         "   <listOptionValue builtIn=\"false\"\n"
         "      value=\"&quot;" << i->string() << "&quot;\" />\n";
   }

   includes_buffer << "</option>\n";

   includes_ = escape_for_regex(includes_buffer.str());
}

void eclipse_cdt_project::write() const
{
   create_directories(workspace_.get_output_path() / project_name_);
   write_eclipse_project_file();
   write_cdt_project_file();
}

std::string eclipse_cdt_project::make_cdt_id()
{
   return boost::lexical_cast<string>(cdt_id_++);
}

void eclipse_cdt_project::write_eclipse_project_file() const
{
   boost::regex pattern("(\\{project_name\\})|"
                        "(\\{build_command\\})|"
                        "(\\{build_location\\})|"
                        "(\\{links\\})|"
                        "(\\{master-tag\\})"
                        );

   string format_string("(?1" + escape_for_regex(project_name_) + ")"
                        "(?2hammer)"
                        "(?3" + escape_for_regex(project_.location().string()) + ")"
                        "(?4" + links_ + ")" +
                        string(is_master_project_ ? "(?5master-project)" : "(?5)")
                        );
   boost::filesystem::ofstream f(workspace_.get_output_path() / project_name_ / ".project", std::ios_base::trunc);
   boost::filesystem::ifstream template_file(workspace_.get_templates_dir() / ".project");
   std::string template_content;
   std::copy(istreambuf_iterator<char>(template_file), istreambuf_iterator<char>(), back_inserter(template_content));
   boost::regex_replace(ostreambuf_iterator<char>(f), 
                        template_content.begin(), 
                        template_content.end(), 
                        pattern, 
                        format_string,
                        boost::match_default | boost::format_all);
}

void eclipse_cdt_project::write_cdt_project_file() const
{
   boost::regex pattern("(\\{configuration_id\\})|"
                        "(\\{toolchain_id\\})|"
                        "(\\{platform_id\\})|"
                        "(\\{builder_id\\})|"
                        "(\\{holder_libs_id\\})|"
                        "(\\{assembler_settings_holder_id\\})|"
                        "(\\{assembler_holder_intype_id\\})|"
                        "(\\{cpp_settings_holder_id\\})|"
                        "(\\{cpp_holder_intype_id\\})|"
                        "(\\{c_settings_holder_id\\})|"
                        "(\\{c_holder_intype_id\\})|"
                        "(\\{project_name\\})|"
                        "(\\{build_command\\})|"
                        "(\\{build_location\\})|"
                        "(\\{project_id\\})|"
                        "(\\{includes\\})|"
                        "(\\{build_arguments\\})"
                        );
   string format_string("(?1 " + configuration_id_ + ")"
                        "(?2 " + toolchain_id_ + ")"
                        "(?3 " + platform_id_ + ")"
                        "(?4 " + builder_id_ + ")"
                        "(?5 " + holder_libs_id_ + ")"
                        "(?6 " + assembler_settings_holder_id_ + ")"
                        "(?7 " + assembler_holder_intype_id_ + ")"
                        "(?8 " + cpp_settings_holder_id_ + ")"
                        "(?9 " + cpp_holder_intype_id_ + ")"
                        "(?10 " + c_settings_holder_id_ + ")"
                        "(?11 " + c_holder_intype_id_ + ")"
                        "(?12 " + escape_for_regex(project_name_) + ")"
                        "(?13hammer)"
                        "(?14" + escape_for_regex(project_.location().string()) + ")"
                        "(?15 " + project_id_ + ")"
                        "(?16" + includes_ + ")"
                        "(?17" + (should_copy_dependencies_ ? "--copy-dependencies mangling=variant" : "") + ")"
                        );

   boost::filesystem::ofstream f(workspace_.get_output_path() / project_name_ / ".cproject", std::ios_base::trunc);
   boost::filesystem::ifstream template_file(workspace_.get_templates_dir() / ".cproject");
   std::string template_content;
   std::copy(istreambuf_iterator<char>(template_file), istreambuf_iterator<char>(), back_inserter(template_content));
   boost::regex_replace(ostreambuf_iterator<char>(f), 
                        template_content.begin(), 
                        template_content.end(), 
                        pattern, 
                        format_string,
                        boost::match_default | boost::format_all);
}

}}
