#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/make_unique.hpp>
#include <boost/crypto/md5.hpp>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/engine.h>
#include <hammer/core/project.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/prebuilt_lib_meta_target.h>
#include <hammer/core/searched_lib_meta_target.h>
#include <hammer/core/cmdline_action.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/feature.h>
#include <hammer/core/alias_meta_target.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/output_location_strategy.h>
#include <hammer/core/rule_argument_types.h>
#include <hammer/core/generated_build_target.h>

using std::string;
using std::unique_ptr;
using std::auto_ptr;
using std::vector;
using namespace boost;
namespace fs = boost::filesystem;

namespace hammer{

typedef boost::function<void(invocation_context& ctx,
                             const parscore::identifier& version,
                             const location_t& root_folder)> rule_t;

const type_tag qt_mocable("QT_MOCABLE");
const type_tag qt_ui("QT_UI");
const type_tag qt_uiced_h("QT_UICED_H");
const type_tag qt_rc("QT_RC");
const type_tag qt_rced_cpp("QT_RCED_CPP");

class qt_uic_meta_target : public typed_meta_target
{
   public:
      qt_uic_meta_target(hammer::project* p,
                         const std::string& name,
                         const sources_decl& sources,
                         const requirements_decl& req,
                         const requirements_decl& usage_req)
                         : typed_meta_target(p, name, req, usage_req, p->get_engine()->get_type_registry().get(qt_uiced_h))
      {
         add_sources(sources);
         set_explicit(true);
      }

   protected:
      void compute_usage_requirements(feature_set& result,
                                      const main_target& constructed_target,
                                      const feature_set& build_request,
                                      const feature_set& computed_usage_requirements,
                                      const main_target* owner) const override;
};

void qt_uic_meta_target::compute_usage_requirements(feature_set& result,
                                                    const main_target& constructed_target,
                                                    const feature_set& build_request,
                                                    const feature_set& computed_usage_requirements,
                                                    const main_target* owner) const
{
   typed_meta_target::compute_usage_requirements(result, constructed_target, build_request, computed_usage_requirements, owner);

   feature* uic_inc = result.owner().create_feature("__generated-include", "");
   uic_inc->get_generated_data().target_ = &constructed_target;

   // making dependency on self :)
   feature* dependency = result.owner().create_feature("dependency", "");
   dependency->set_dependency_data(source_decl(name(), std::string(), nullptr, nullptr), this);

   result.join(uic_inc).join(dependency);
}

class qt_uic_generator : public generator
{
   public:
      qt_uic_generator(engine& e,
                       const build_action_ptr& action,
                       const feature_set* c = nullptr)
         : generator(e, "qt.uic", make_consume_types(e, { qt_ui }), make_product_types(e, { qt_uiced_h }), true, action, c)
      {
      }

      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const build_nodes_t& sources,
                const basic_build_target* source_target,
                const std::string* composite_target_name,
                const main_target& owner) const override
      {
         const feature_set* valuable_properties = make_valuable_properties(props, type_to_construct.valuable_features(), {});
         build_nodes_t result;

         for (auto& source : sources) {
            for (auto& product_from_source : source->products_) {
               if (is_consumable(product_from_source->type())) {
                  build_node_ptr one_node(new build_node(owner, false, action()));

                  one_node->sources_.push_back(build_node::source_t(product_from_source, source));
                  one_node->products_.push_back(create_target(&owner, one_node->sources_, nullptr, producable_types().front(), valuable_properties));
                  one_node->targeting_type_ = &type_to_construct;

                  result.push_back(one_node);
               }
            }
         }

         return result;
      }
};

static
unique_ptr<sources_decl>
qt_uic_rule(invocation_context& ctx,
            const sources_decl& sources)
{
   std::stringstream s;
   sources.dump_for_hash(s);

   auto_ptr<basic_meta_target> mt(new qt_uic_meta_target(&ctx.current_project_,
                                                         "qt.uic." + boost::crypto::md5(s.str()).to_string(),
                                                         sources,
                                                         requirements_decl(),
                                                         requirements_decl()));

   auto result = boost::make_unique<sources_decl>();
   result->push_back(source_decl("./", mt->name(), NULL, NULL));

   ctx.current_project_.add_target(mt);

   return result;
}

static sources_decl convert_H_to_MOCABLE(const sources_decl& src, hammer::project& p)
{
   sources_decl result;
   const target_type& h_type = p.get_engine()->get_type_registry().get(types::H);
   const target_type& qt_mocable_type = p.get_engine()->get_type_registry().get(qt_mocable);
   for(sources_decl::const_iterator i = src.begin(), last = src.end(); i != last; ++i)
   {
      if (i->type() != NULL &&
          i->type()->equal_or_derived_from(h_type))
      {
         source_decl s(*i);
         s.set_type(&qt_mocable_type);
         result.push_back(s);
      }
   }

   return result;
}

class qt_moc_meta_target : public alias_meta_target
{
   public:
      qt_moc_meta_target(hammer::project* p,
                         const std::string& name,
                         const sources_decl& sources,
                         const requirements_decl& req,
                         const requirements_decl& usage_req)
         : alias_meta_target(p, name, convert_H_to_MOCABLE(sources, *p), req, usage_req)
      {

      }
};

void qt_moc_rule(target_invocation_context& ctx,
                 const parscore::identifier& id,
                 const sources_decl& sources,
                 requirements_decl* requirements,
                 const feature_set* default_build,
                 requirements_decl* usage_requirements)

{
   auto_ptr<basic_meta_target> mt(new qt_moc_meta_target(&ctx.current_project_,
                                                         id.to_string(),
                                                         sources,
                                                         requirements ? *requirements : requirements_decl(),
                                                         usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

qt_toolset::qt_toolset()
   : toolset("qt",
             rule_manager::make_rule_declaration("use-toolset-qt",
                                                 rule_t(boost::bind(&qt_toolset::use_toolset_rule, this, _1, _2, _3)),
                                                 {"version", "root"}))
{
}

static
string determinate_version(const location_t& toolset_home)
{
   for(fs::directory_iterator i = fs::directory_iterator(toolset_home), last = fs::directory_iterator(); i != last; ++i)
   {
      boost::smatch m;
      string filename = i->path().filename().string();
      if (boost::regex_match(filename, m, boost::regex("changes-([0-9\\.]+)")))
         return m[1];
   }

   return string();
}

void qt_toolset::autoconfigure(engine& e) const
{
//    const char* qt_dir = getenv("QTDIR");
//    if (qt_dir != NULL) {
//      location_t toolset_home(qt_dir);
//      string version = determinate_version(toolset_home);
//      if (version.empty())
//         throw std::runtime_error("Can't determinate version for Qt toolset at '" + toolset_home.string<string>() + "'");

//      init_impl(e, version, &toolset_home);
//    } else if (fs::exists("/usr/include/qt4/Qt/QtCore")) {
//       // FIXME: linux part
//       location_t toolset_home("/usr");
//       string version = "4";

//       init_impl(e, version, &toolset_home);
//    }
}

void qt_toolset::configure(engine& e,
                           const std::string& version) const
{
   throw std::runtime_error("Qt toolset cannot be simply configured specifying version");
}

static
void add_lib(project& qt_project,
             const string& lib_name,
             const vector<string>& dependencies,
             engine& e,
             const string& include_tag,
             const string& additional_include_path,
             const string& lib_tag)
{
   requirements_decl debug_req;

   feature* qt_no_debug_feature = e.feature_registry().create_feature("define", "QT_NO_DEBUG");
#if defined(_WIN32)
   feature* top_include_feature = e.feature_registry().create_feature("include", "./include");
   feature* include_feature = e.feature_registry().create_feature("include", "./include/" + additional_include_path);
#else
   feature* top_include_feature = e.feature_registry().create_feature("include", "./include/" + include_tag);
   feature* include_feature = e.feature_registry().create_feature("include", "./include/" + include_tag + "/" + additional_include_path);
#endif
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

   requirements_decl release_req;
   {
      auto_ptr<just_feature_requirement> include_req(new just_feature_requirement(include_feature));
      include_req->set_public(true);
      release_req.add(auto_ptr<requirement_base>(include_req));
   }
   {
      auto_ptr<just_feature_requirement> top_include_req(new just_feature_requirement(top_include_feature));
      top_include_req->set_public(true);
      release_req.add(auto_ptr<requirement_base>(top_include_req));
   }
   {
      auto_ptr<just_feature_requirement> qt_no_debug(new just_feature_requirement(qt_no_debug_feature));
      qt_no_debug->set_public(true);
      release_req.add(auto_ptr<requirement_base>(qt_no_debug));
   }
   release_req.add(auto_ptr<requirement_base>(new just_feature_requirement(e.feature_registry().create_feature("variant", "release"))));

   requirements_decl profile_req;
   {
      auto_ptr<just_feature_requirement> include_req(new just_feature_requirement(include_feature));
      include_req->set_public(true);
      profile_req.add(auto_ptr<requirement_base>(include_req));
   }
   {
      auto_ptr<just_feature_requirement> top_include_req(new just_feature_requirement(top_include_feature));
      top_include_req->set_public(true);
      profile_req.add(auto_ptr<requirement_base>(top_include_req));
   }
   profile_req.add(auto_ptr<requirement_base>(new just_feature_requirement(e.feature_registry().create_feature("variant", "profile"))));
#if defined(_WIN32)
   auto_ptr<prebuilt_lib_meta_target> lib_debug(
         new prebuilt_lib_meta_target(&qt_project,
                               lib_name,
                               "./lib/" + lib_name + "d" + lib_tag + ".lib", debug_req, requirements_decl()));
   auto_ptr<prebuilt_lib_meta_target> lib_release(
         new prebuilt_lib_meta_target(&qt_project,
                               lib_name,
                               "./lib/" + lib_name + lib_tag + ".lib", release_req, requirements_decl()));
   auto_ptr<prebuilt_lib_meta_target> lib_profile(
         new prebuilt_lib_meta_target(&qt_project,
                               lib_name,
                               "./lib/" + lib_name + lib_tag + ".lib", profile_req, requirements_decl()));
#else
   feature* search_feature = e.feature_registry().create_feature("search", "./lib/");
   search_feature->get_path_data().target_ = &qt_project;
   {
      auto_ptr<just_feature_requirement> search_req(new just_feature_requirement(search_feature));
      debug_req.add(auto_ptr<requirement_base>(search_req));
   }
   auto_ptr<searched_lib_meta_target> lib_debug(
         new searched_lib_meta_target(&qt_project,
                                      lib_name,
                                      lib_name,
                                      debug_req,
                                      requirements_decl(),
                                      e.get_type_registry().get(types::SEARCHED_SHARED_LIB)));

   {
      auto_ptr<just_feature_requirement> search_req(new just_feature_requirement(search_feature));
      release_req.add(auto_ptr<requirement_base>(search_req));
   }
   auto_ptr<searched_lib_meta_target> lib_release(
         new searched_lib_meta_target(&qt_project,
                                      lib_name,
                                      lib_name,
                                      release_req,
                                      requirements_decl(),
                                      e.get_type_registry().get(types::SEARCHED_SHARED_LIB)));
   {
      auto_ptr<just_feature_requirement> search_req(new just_feature_requirement(search_feature));
      profile_req.add(auto_ptr<requirement_base>(search_req));
   }
   auto_ptr<searched_lib_meta_target> lib_profile(
         new searched_lib_meta_target(&qt_project,
                                      lib_name,
                                      lib_name,
                                      profile_req,
                                      requirements_decl(),
                                      e.get_type_registry().get(types::SEARCHED_SHARED_LIB)));
#endif

   for(size_t i = 0; i < dependencies.size(); ++i)
   {
      requirements_decl usage_req;
      feature* source_feature = e.feature_registry().create_feature("source", dependencies[i]);
      {
         source_decl sd("/Qt", dependencies[i], NULL, e.feature_registry().make_set());
         source_feature->set_dependency_data(sd, &qt_project);
         auto_ptr<just_feature_requirement> source_req(new just_feature_requirement(source_feature));
         usage_req.add(auto_ptr<requirement_base>(source_req));
      }

      lib_debug->usage_requirements().insert(usage_req);
      lib_release->usage_requirements().insert(usage_req);
      lib_profile->usage_requirements().insert(usage_req);
   }

   qt_project.add_target(auto_ptr<basic_meta_target>(lib_debug));
   qt_project.add_target(auto_ptr<basic_meta_target>(lib_release));
   qt_project.add_target(auto_ptr<basic_meta_target>(lib_profile));
}

static
void add_libs_and_generators(engine& e,
                             const location_t* toolset_home,
                             const std::string& bin_tag,
                             const string& lib_tag,
                             const string& include_tag,
                             const bool qt5)
{
   // QT_UI -> QT_UICED_H
   {
      shared_ptr<source_argument_writer> ui_source(new source_argument_writer("ui_source", e.get_type_registry().get(qt_ui)));
      shared_ptr<product_argument_writer> uic_product(new product_argument_writer("uic_product", e.get_type_registry().get(types::H)));
      cmdline_builder uic_cmd((*toolset_home / ("bin/uic" + bin_tag)).string<string>() + " -o \"$(uic_product)\" $(ui_source)");

      uic_cmd += ui_source;
      uic_cmd += uic_product;

      auto uic_action = std::make_shared<cmdline_action>("qt.uic", uic_product);
      *uic_action += uic_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(qt_ui), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::H)));
//      unique_ptr<generator> g(new generator(e, "qt.uic", source, target, true, uic_action));
      unique_ptr<generator> g(new qt_uic_generator(e, uic_action));
      e.generators().insert(std::move(g));
   }

   // QT_RC -> QT_RCED_CPP
   {
      shared_ptr<source_argument_writer> rcc_source(new source_argument_writer("rcc_source", e.get_type_registry().get(qt_rc)));
      shared_ptr<product_argument_writer> rcc_product(new product_argument_writer("rcc_product", e.get_type_registry().get(qt_rced_cpp)));
      cmdline_builder rcc_cmd((*toolset_home / ("bin/rcc" + bin_tag)).string<string>() + " -o \"$(rcc_product)\" $(rcc_source)");

      rcc_cmd += rcc_source;
      rcc_cmd += rcc_product;

      auto rcc_action = std::make_shared<cmdline_action>("qt.rcc", rcc_product);
      *rcc_action += rcc_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(qt_rc), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(qt_rced_cpp)));
      unique_ptr<generator> g(new generator(e, "qt.rcc", source, target, false, rcc_action));
      e.generators().insert(std::move(g));
   }

   // QT_MOCABLE -> CPP
   {
      shared_ptr<source_argument_writer> mocable_source(new source_argument_writer("mocable_source", e.get_type_registry().get(qt_mocable)));
      shared_ptr<product_argument_writer> cpp_product(new product_argument_writer("cpp_product", e.get_type_registry().get(types::CPP)));
      cmdline_builder moc_cmd((*toolset_home / ("bin/moc" + bin_tag)).string<string>() + " -o \"$(cpp_product)\" $(mocable_source)");

      moc_cmd += mocable_source;
      moc_cmd += cpp_product;

      auto moc_action = std::make_shared<cmdline_action>("qt.moc", cpp_product);
      *moc_action += moc_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(qt_mocable), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::CPP)));
      unique_ptr<generator> g(new generator(e, "qt.moc", source, target, false, moc_action));
      e.generators().insert(std::move(g));
   }

   // register qt libs
   auto_ptr<project> qt_project(new project(&e, "Qt", *toolset_home, requirements_decl(), requirements_decl()));
   if (qt5) {
      add_lib(*qt_project, "Qt5Core", vector<string>(), e, include_tag, "QtCore", lib_tag);
      add_lib(*qt_project, "Qt5Gui", {"Qt5Core"}, e, include_tag, "QtGui", lib_tag);
      add_lib(*qt_project, "Qt5Widgets", {"Qt5Core", "Qt5Gui"}, e, include_tag, "QtWidgets", lib_tag);
   } else {
      add_lib(*qt_project, "QtCore", vector<string>(), e, include_tag, "QtCore", lib_tag);
      add_lib(*qt_project, "QtGui", {"QtCore"}, e, include_tag, "QtGui", lib_tag);
   }

   e.insert(qt_project.get());
   e.use_project(*qt_project, "/Qt", "");
   qt_project.release();
}

void qt_toolset::use_toolset_rule(invocation_context& ctx,
                                  const parscore::identifier& version,
                                  const location_t& root_folder)
{
   engine& e = *ctx.current_project_.get_engine();

   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value("qt")) {
      toolset_def.extend_legal_values("qt", e.feature_registry().get_or_create_feature_value_ns("qt"));

      e.get_rule_manager().add_target("qt.moc", qt_moc_rule, { "id", "sources", "requirements", "default-build", "usage-requirements" });
      e.get_rule_manager().add_rule("qt.uic", qt_uic_rule, { "sources" });

      // register qt types
      // FIXME: we don't derive qt mocable from H because current generator implementation can't properly choose qt.moc generator
      e.get_type_registry().insert(target_type(qt_mocable, ""));
      e.get_type_registry().insert(target_type(qt_ui, ".ui"));
      e.get_type_registry().insert(target_type(qt_rc, ".qrc"));
      e.get_type_registry().insert(target_type(qt_rced_cpp, ".cpp", e.get_type_registry().get(types::CPP), "qrc_"));
      e.get_type_registry().insert(target_type(qt_uiced_h, ".h", e.get_type_registry().get(types::H), "ui_"));
   }

   const string s_version=version.to_string();
   if (!s_version.empty() && s_version[0] == '4')
      add_libs_and_generators(e, &root_folder, "-qt4", "4", "qt4", false);
   else
      add_libs_and_generators(e, &root_folder, "", "", "", true);
}

//void qt_toolset::init_impl(engine& e,
//                           const std::string& version_id,
//                           const location_t* toolset_home) const
//{
//   if (e.get_type_registry().find(qt_mocable) == NULL) {
//      if (!version_id.empty() && version_id[0] == '4') {
//         add_types_and_generators(e, toolset_home, "-qt4", "4", "qt4", false);

//         feature_def& toolset_def = e.feature_registry().get_def("toolset");
//         if (!toolset_def.is_legal_value("qt"))
//            toolset_def.extend_legal_values("qt", e.feature_registry().get_or_create_feature_value_ns("qt"));

//         return;
//      }

//      if (toolset_home) {
//         add_types_and_generators(e, toolset_home, "", "", "", true);

//         feature_def& toolset_def = e.feature_registry().get_def("toolset");
//         if (!toolset_def.is_legal_value("qt"))
//            toolset_def.extend_legal_values("qt", e.feature_registry().get_or_create_feature_value_ns("qt"));

//         return;
//      }
//   }

//   throw std::runtime_error("Don't know how to configure Qt toolset version '" + version_id + "'");
//}

}
