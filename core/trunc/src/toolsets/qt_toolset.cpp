#include "stdafx.h"
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/engine.h>
#include <hammer/core/project.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/prebuilt_lib_meta_target.h>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>
#include <hammer/core/cmdline_action.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/generator.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/feature.h>

using namespace std;
using namespace boost;
using namespace boost::assign;
namespace fs = boost::filesystem;

namespace hammer{

qt_toolset::qt_toolset()
: toolset("qt")
{
}

static string determinate_version(const location_t& toolset_home)
{
	for(fs::directory_iterator i = fs::directory_iterator(toolset_home), last = fs::directory_iterator(); i != last; ++i)
	{
		boost::smatch m;
		string filename = i->path().filename();
      if (boost::regex_match(filename, m, boost::regex("changes-([0-9\\.]+)")))
			return m[1];
	}

	return string();
}

void qt_toolset::autoconfigure(engine& e) const
{
    const char* qt_dir = getenv("QTDIR");
    if (qt_dir != NULL)
    {
    	location_t toolset_home(qt_dir);
    	string version = determinate_version(toolset_home);
    	if (version.empty())
    		throw std::runtime_error("Can't determinate version for Qt toolset at '" + toolset_home.native_file_string() + "'");

    	init_impl(e, version, &toolset_home);
    }
}

const type_tag qt_mockable("QT_MOCKABLE");
const type_tag qt_ui("QT_UI");
const type_tag qt_rc("QT_RC");

static void add_lib(project& qt_project, const string& lib_name, const vector<string>& dependencies, engine& e)
{
	requirements_decl debug_req;
	feature* top_include_feature = e.feature_registry().create_feature("include", "./include");
	feature* include_feature = e.feature_registry().create_feature("include", "./include/" + lib_name);
	{
		auto_ptr<just_feature_requirement> include_req(new just_feature_requirement(include_feature));
		include_req->set_public(true);
		debug_req.add(auto_ptr<requirement_base>(include_req));
	}
	{
		auto_ptr<just_feature_requirement> top_include_req(new just_feature_requirement(top_include_feature));
		top_include_req->set_public(true);
		debug_req.add(auto_ptr<requirement_base>(top_include_req));
	}
	debug_req.add(auto_ptr<requirement_base>(new just_feature_requirement(e.feature_registry().create_feature("variant", "debug"))));

	requirements_decl  release_or_profile_req;
	{
		auto_ptr<just_feature_requirement> include_req(new just_feature_requirement(include_feature));
		include_req->set_public(true);
		release_or_profile_req.add(auto_ptr<requirement_base>(include_req));
	}
	{
		auto_ptr<just_feature_requirement> top_include_req(new just_feature_requirement(top_include_feature));
		top_include_req->set_public(true);
		release_or_profile_req.add(auto_ptr<requirement_base>(top_include_req));
	}

	auto_ptr<prebuilt_lib_meta_target> lib_debug(
			new prebuilt_lib_meta_target(&qt_project,
										 pstring(e.pstring_pool(), lib_name),
										 pstring(e.pstring_pool(), "./lib/" + lib_name + "d4.lib"), debug_req, requirements_decl()));
	auto_ptr<prebuilt_lib_meta_target> lib_non_debug(
			new prebuilt_lib_meta_target(&qt_project,
										 pstring(e.pstring_pool(), lib_name),
										 pstring(e.pstring_pool(), "./lib/" + lib_name + "4.lib"), release_or_profile_req, requirements_decl()));

	for(size_t i = 0; i < dependencies.size(); ++i)
	{
	   requirements_decl usage_req;
	   feature* source_feature = e.feature_registry().create_feature("source", dependencies[i]);
	   {
	      source_decl sd(pstring(e.pstring_pool(), "/Qt"),
	                     pstring(e.pstring_pool(), dependencies[i]),
	                     NULL,
	                     e.feature_registry().make_set());
	      source_feature->get_dependency_data().source_ = sd;
	      auto_ptr<just_feature_requirement> source_req(new just_feature_requirement(source_feature));
	      usage_req.add(auto_ptr<requirement_base>(source_req));
	   }

	   lib_debug->usage_requirements().insert(usage_req);
      lib_non_debug->usage_requirements().insert(usage_req);
	}

	qt_project.add_target(auto_ptr<basic_meta_target>(lib_debug));
	qt_project.add_target(auto_ptr<basic_meta_target>(lib_non_debug));
}

void qt_toolset::init_impl(engine& e, const std::string& version_id,
                           const location_t* toolset_home) const
{
	if (e.get_type_registry().find(qt_mockable) == NULL)
	{
		// register qt types
		e.get_type_registry().insert(target_type(qt_mockable, "", e.get_type_registry().get(types::H)));
		e.get_type_registry().insert(target_type(qt_ui, ".ui"));
		e.get_type_registry().insert(target_type(qt_rc, ".qrc"));

		// UI -> CPP
      shared_ptr<source_argument_writer> ui_source(new source_argument_writer("ui_source", e.get_type_registry().get(qt_ui)));
      shared_ptr<product_argument_writer> cpp_product(new product_argument_writer("cpp_product", e.get_type_registry().get(types::CPP)));
      cmdline_builder res_cmd((*toolset_home / "uic").native_file_string() + " -o\"$(cpp_product)\" $(ui_source)");

      res_cmd += ui_source;
      res_cmd += cpp_product;

      auto_ptr<cmdline_action> res_action(new cmdline_action("qt.uic", cpp_product));
      *res_action += res_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(qt_ui), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::CPP)));
      auto_ptr<generator> g(new generator(e, "qt.uic", source, target, false));
      g->action(res_action);
      e.generators().insert(g);

		// register qt libs
		auto_ptr<project> qt_project(new project(&e,
                                               pstring(e.pstring_pool(), "Qt"),
                                               *toolset_home,
                                               requirements_decl(),
                                               requirements_decl()));
		add_lib(*qt_project, "QtCore", vector<string>(), e);
		add_lib(*qt_project, "QtGui", list_of("QtCore"), e);
		e.insert(qt_project.get());
		e.use_project(*qt_project, pstring(e.pstring_pool(), "/Qt"), "");
		qt_project.release();
	}
}

}
