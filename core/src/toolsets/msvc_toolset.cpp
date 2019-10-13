#include <boost/filesystem/convenience.hpp>
#include <boost/make_unique.hpp>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/static_lib_generator.h>
#include <hammer/core/exe_and_shared_lib_generator.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/pch_generator.h>
#include <hammer/core/cmdline_action.h>
#include <hammer/core/fs_argument_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/free_feature_arg_writer.h>
#include <hammer/core/pch_argument_writer.h>
#include <hammer/core/output_dir_argument_writer.h>
#include <hammer/core/batched_cmdline_action.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/ast2objects.h>

using std::string;
using std::unique_ptr;
using namespace boost;

namespace hammer {

namespace {

struct msvc_data {
   location_t setup_script_;
   location_t compiler_;
   location_t linker_;
   location_t librarian_;
   location_t manifest_tool_;
   location_t resource_compiler_;
};

}

msvc_toolset::msvc_toolset()
   : toolset("msvc",
             rule_manager::make_rule_declaration("use-toolset-msvc",
                                                 this, &msvc_toolset::use_toolset_rule,
                                                 {"version", "path-to-vc-dir"}))
{
}

static
YAML::Node
make_toolset_info(const msvc_data& td) {
   YAML::Node info;

   info["setup-script"] = td.setup_script_.string();
   info["compiler"] = td.compiler_.string();
   info["linker"] = td.linker_.string();
   info["librarian"] = td.librarian_.string();
   info["manifest-tool"] = td.manifest_tool_.string();
   info["resource-compiler"] = td.resource_compiler_.string();

   return info;
}

void msvc_toolset::init_toolset(engine& e,
                                const std::string& version_id,
                                const location_t& toolset_home) {
   const feature_set& constraints = *e.feature_registry().make_set();
   if (is_already_configured(version_id, constraints))
      throw std::runtime_error("Version '" + version_id + "' already configured");

   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value(name()))
      toolset_def.extend_legal_values(name(), e.feature_registry().get_or_create_feature_value_ns("c/c++"));

   toolset_def.get_subfeature("version").extend_legal_values(name(), version_id);

   msvc_data config_data;
   config_data.setup_script_ = toolset_home / "bin/vcvars32.bat";
   config_data.compiler_ = "cl.exe";
   config_data.librarian_ = "lib.exe";
   config_data.linker_ = "link.exe";
   config_data.manifest_tool_ = "mt.exe";
   config_data.resource_compiler_ = "rc.exe";

   if (!e.feature_registry().find_def("debug-store")) {
      feature_attributes fa = {0};
      fa.propagated = true; //FIXME: why?
      e.feature_registry().add_feature_def("debug-store", { {"database"}, {"object"} }, fa);
   }

   feature_set* generator_condition = e.feature_registry().make_set();
   if (!version_id.empty())
      generator_condition->join("toolset", (name() + '-' + version_id).c_str());
   else
      generator_condition->join("toolset", name().c_str());

   cmdline_builder setup_vars("call \"" + config_data.setup_script_.string() + "\" >nul");
   std::shared_ptr<source_argument_writer> static_lib_sources(new source_argument_writer("static_lib_sources", e.get_type_registry().get(types::STATIC_LIB), true, source_argument_writer::FULL_PATH));
   std::shared_ptr<source_argument_writer> searched_lib_sources(new source_argument_writer("searched_lib_sources", e.get_type_registry().get(types::SEARCHED_LIB), false, source_argument_writer::WITHOUT_PATH));
   std::shared_ptr<source_argument_writer> prebuilt_lib_sources(new source_argument_writer("prebuilt_lib_sources", e.get_type_registry().get(types::PREBUILT_STATIC_LIB), true, source_argument_writer::FULL_PATH));
   std::shared_ptr<source_argument_writer> import_lib_sources(new source_argument_writer("import_lib_sources", e.get_type_registry().get(types::IMPORT_LIB), true, source_argument_writer::FULL_PATH));

   std::shared_ptr<product_argument_writer> obj_product(new product_argument_writer("obj_product", e.get_type_registry().get(types::OBJ)));
   std::shared_ptr<product_argument_writer> pch_product(new product_argument_writer("pch_product", e.get_type_registry().get(types::PCH)));

   std::shared_ptr<free_feature_arg_writer> searched_lib_searched_dirs(
      new free_feature_arg_writer("searched_lib_searched_dirs", 
                                  e.feature_registry(),
                                  "search",
                                  string(), 
                                  string(), 
                                  ";",
                                  "/LIBPATH:\"",
                                  "\""));

   std::shared_ptr<fs_argument_writer> link_flags(new fs_argument_writer("link_flags", e.feature_registry()));
   link_flags->add("<debug-symbols>on", "/DEBUG /INCREMENTAL").
               add("<debug-symbols>on/<runtime-debugging>off", "/OPT:REF,ICF").
               add("<user-interface>console", "/subsystem:console").
               add("<user-interface>gui", "/subsystem:windows").
               add("<user-interface>gui/<character-set>unicode", "/ENTRY:\"wWinMainCRTStartup\"").
               add("<user-interface>wince", "/subsystem:windowsce").
               add("<user-interface>native", "/subsystem:native").
               add("<user-interface>auto", "/subsystem:posix");

   std::shared_ptr<fs_argument_writer> cflags(new fs_argument_writer("cflags", e.feature_registry()));
   cflags->add("<optimization>speed", "/O2").
           add("<optimization>space", "/O1").
           add("<optimization>off", "/Od").
           add("<debug-symbols>on/<debug-store>object", "/Z7").
           add("<debug-symbols>on/<debug-store>database", "/Zi").
           add("<inlining>off", "/Ob0").
           add("<inlining>on", "/Ob1").
           add("<inlining>full", "/Ob2").
           add("<warnings>on", "/W3").
           add("<warnings>off", "/W0").
           add("<warnings>all", "/W4").
           add("<warnings-as-errors>on", "/WX").
           add("<rtti>on","/GR").
           add("<character-set>unicode", "/D \"UNICODE\" /D \"_UNICODE\"").
           add("<runtime-debugging>off/<runtime-link>static/<threading>multi", "/MT").
           add("<runtime-debugging>on/<runtime-link>static/<threading>multi", "/MTd").
           add("<runtime-debugging>off/<runtime-link>shared", "/MD").
           add("<runtime-debugging>on/<runtime-link>shared", "/MDd");

   std::shared_ptr<fs_argument_writer> cppflags(new fs_argument_writer("cppflags", e.feature_registry()));
   cppflags->add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>off", "/EHs").
             add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>on", "/EHsc").
             add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>off", "/EHa").
             add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>on", "/EHac");

   std::shared_ptr<free_feature_arg_writer> user_c_flags(new free_feature_arg_writer("user_c_flags", e.feature_registry(), "cflags"));
   std::shared_ptr<free_feature_arg_writer> user_link_flags(new free_feature_arg_writer("user_link_flags", e.feature_registry(), "linkflags"));
   std::shared_ptr<free_feature_arg_writer> user_cxx_flags(new free_feature_arg_writer("user_cxx_flags", e.feature_registry(), "cxxflags"));
   std::shared_ptr<free_feature_arg_writer> user_archive_flags(new free_feature_arg_writer("user_archive_flags", e.feature_registry(), "archiveflags"));

   std::shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry(), "include", "-I \"", "\""));
   std::shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry(), "define", "-D \"", "\""));
   std::shared_ptr<free_feature_arg_writer> undefines(new free_feature_arg_writer("undefines", e.feature_registry(), "undef", "-U \"", "\""));

   std::shared_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP), /*exact_type=*/false));
   std::shared_ptr<source_argument_writer> res_sources(new source_argument_writer("res_sources", e.get_type_registry().get(types::RES)));
   std::shared_ptr<pch_argument_writer> create_pch_header(new pch_argument_writer("create_pch_header", pch_argument_writer::part::header,
                                                                                  "/Yc\"", "\""));
   std::shared_ptr<pch_argument_writer> use_pch_header(new pch_argument_writer("use_pch_header", pch_argument_writer::part::header,
                                                                               "/Yu\"", "\""));
   std::shared_ptr<pch_argument_writer> use_pch_product(new pch_argument_writer("use_pch_product", pch_argument_writer::part::product,
                                                                                "/Fp\"", "\""));
   std::shared_ptr<output_dir_argument_writer> output_dir(new output_dir_argument_writer("output_dir"));

   const string generator_prefix = name() + "-" + version_id;

   // CPP -> OBJ
   {
      cmdline_builder obj_cmd(config_data.compiler_.string() +
                              " /c /TP /nologo $(cflags) $(cppflags) $(user_cxx_flags) $(includes) $(undefines) $(defines) $(use_pch_header) $(use_pch_product) $(cpp_input)"
                              " /Fo\"$(obj_product)\" /FS /Fd\"$(output_dir)\\vc.pdb\"");

      cmdline_builder batched_obj_cmd(config_data.compiler_.string() +
                                      " /c /nologo $(cflags) $(cppflags) $(user_cxx_flags) $(includes) $(undefines) $(defines) $(use_pch_header) $(use_pch_product) $(cpp_input)"
                                      " /Fo\"$(output_dir)\\\\\" /FS /Fd\"$(output_dir)\\vc.pdb\"");
      obj_cmd += cflags;
      obj_cmd += cppflags;
      obj_cmd += user_cxx_flags;
      obj_cmd += cpp_input;
      obj_cmd += includes;
      obj_cmd += undefines;
      obj_cmd += defines;
      obj_cmd += use_pch_header;
      obj_cmd += use_pch_product;
      obj_cmd += output_dir;
      
      batched_obj_cmd.writers(obj_cmd.writers());

      obj_cmd += obj_product;

      auto obj_action = std::make_shared<cmdline_action>("compile-c++", obj_product);
      *obj_action += setup_vars;
      *obj_action += obj_cmd;

      std::shared_ptr<batched_cmdline_action> batched_obj_action(new batched_cmdline_action("batched compile-c++"));
      *batched_obj_action += setup_vars;
      *batched_obj_action += batched_obj_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));

      add_compile_generators(e, obj_action, [&](const build_action_ptr& compile_action) {
         return make_unique<generator>(e, generator_prefix + ".compiler.cpp", source, target, false, compile_action, generator_condition);
      });
   }
   
   // CPP + H -> PCH + OBJ
   {
      cmdline_builder obj_cmd(config_data.compiler_.string() +
                              " /c /TP /nologo $(create_pch_header) $(cflags) $(cppflags) $(user_cxx_flags) $(includes) $(undefines) $(defines) $(cpp_input)"
                              " /Fo\"$(obj_product)\" /Fp\"$(pch_product)\" /FS /Fd\"$(output_dir)\\vc.pdb\"");
      obj_cmd += cflags;
      obj_cmd += cppflags;
      obj_cmd += user_cxx_flags;
      obj_cmd += cpp_input;
      obj_cmd += create_pch_header;
      obj_cmd += includes;
      obj_cmd += undefines;
      obj_cmd += defines;
      obj_cmd += obj_product;
      obj_cmd += pch_product;
      obj_cmd += output_dir;
      auto obj_action = std::make_shared<cmdline_action>("compile-c++-pch", obj_product);
      *obj_action += setup_vars;
      *obj_action += obj_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::PCH)));

      feature_set* constraints = e.feature_registry().make_set();
      constraints->join("__pch", "");
      constraints->join(*generator_condition);
      unique_ptr<generator> g(new pch_generator(e, generator_prefix + ".compiler.pch", source, target, true, obj_action, constraints));
      e.generators().insert(std::move(g));
   }

   // C -> OBJ
   {
      std::shared_ptr<source_argument_writer> c_source(new source_argument_writer("c_source", e.get_type_registry().get(types::C)));
      cmdline_builder obj_cmd(config_data.compiler_.string() +
                              " /c /TC /nologo $(cflags) $(user_c_flags) $(includes) $(undefines) $(defines) $(c_source) /Fo\"$(obj_product)\" /FS /Fd\"$(output_dir)\\vc.pdb\"");
      obj_cmd += cflags;
      obj_cmd += user_c_flags;
      obj_cmd += c_source;
      obj_cmd += includes;
      obj_cmd += undefines;
      obj_cmd += defines;
      obj_cmd += obj_product;
      obj_cmd += output_dir;

      auto obj_action = std::make_shared<cmdline_action>("compile-c", obj_product);
      *obj_action += setup_vars;
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      add_compile_generators(e, obj_action, [&](const build_action_ptr& compile_action) {
         return make_unique<generator>(e, generator_prefix + ".compiler.c", source, target, false, compile_action, generator_condition);
      });
   }

   // RC -> RES
   {
      std::shared_ptr<source_argument_writer> rc_source(new source_argument_writer("rc_source", e.get_type_registry().get(types::RC)));
      std::shared_ptr<product_argument_writer> res_product(new product_argument_writer("res_product", e.get_type_registry().get(types::RES)));
      cmdline_builder res_cmd(config_data.resource_compiler_.string() + " $(includes) $(undefines) $(defines) /Fo\"$(res_product)\" $(rc_source)");

      res_cmd += rc_source;
      res_cmd += includes;
      res_cmd += undefines;
      res_cmd += defines;
      res_cmd += res_product;

      auto res_action = std::make_shared<cmdline_action>("compile-rc", res_product);
      *res_action += setup_vars;
      *res_action += res_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::RC), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::RES)));
      unique_ptr<generator> g(new generator(e, generator_prefix + ".compiler.rc", source, target, false, res_action, generator_condition));
      e.generators().insert(std::move(g));
   }

   // ... -> EXE
   { 
      std::shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      std::shared_ptr<product_argument_writer> exe_product_unc(new product_argument_writer("exe_product_unc", e.get_type_registry().get(types::EXE),
                                                                                            product_argument_writer::output_strategy::FULL_PATH));
      std::shared_ptr<product_argument_writer> exe_product(new product_argument_writer("exe_product", e.get_type_registry().get(types::EXE)));
      cmdline_builder exe_cmd(config_data.linker_.string() + " \"@$(exe_product).rsp\"");

      exe_cmd += exe_product;
      cmdline_builder exe_rsp("  /nologo $(link_flags) $(user_link_flags) $(searched_lib_searched_dirs) /out:\"$(exe_product_unc)\" $(obj_sources) $(res_sources) $(static_lib_sources) $(prebuilt_lib_sources) $(searched_lib_sources) $(import_lib_sources)");

      exe_rsp += link_flags;
      exe_rsp += user_link_flags;
      exe_rsp += searched_lib_searched_dirs;
      exe_rsp += obj_sources;
      exe_rsp += res_sources;
      exe_rsp += static_lib_sources;
      exe_rsp += prebuilt_lib_sources;
      exe_rsp += searched_lib_sources;
      exe_rsp += import_lib_sources;
      exe_rsp += exe_product_unc;

      auto exe_action = std::make_shared<cmdline_action>("link-exe", exe_product, exe_rsp);
      *exe_action += setup_vars;
      *exe_action += exe_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::RES), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE)));

      add_link_generators(e, exe_action, [&](const build_action_ptr& link_action) {
         return make_unique<exe_and_shared_lib_generator>(e, generator_prefix + ".linker.exe", source, target, true, link_action, generator_condition, nullptr);
      });
   }

   // ... -> STATIC_LIB
   { 
      std::shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      std::shared_ptr<product_argument_writer> static_lib_product(new product_argument_writer("static_lib_product", e.get_type_registry().get(types::STATIC_LIB)));
      std::shared_ptr<product_argument_writer> static_lib_product_unc(new product_argument_writer("static_lib_product_unc",
                                                                                                  e.get_type_registry().get(types::STATIC_LIB),
                                                                                                  product_argument_writer::output_strategy::FULL_PATH));
      cmdline_builder static_lib_rsp("$(user_archive_flags) /out:\"$(static_lib_product)\"\n$(obj_sources)");
      static_lib_rsp += user_archive_flags;
      static_lib_rsp += obj_sources;
      static_lib_rsp += static_lib_product;
      cmdline_builder static_lib_cmd("if exist \"$(static_lib_product)\" DEL \"$(static_lib_product)\"\n" +
                                     config_data.librarian_.string() + " /nologo \"@$(static_lib_product_unc).rsp\"");
      
      static_lib_cmd += static_lib_product;
      static_lib_cmd += static_lib_product_unc;
      static_lib_cmd += obj_sources;
      auto static_lib_action = std::make_shared<cmdline_action>("link-static-lib", static_lib_product, static_lib_rsp);
      *static_lib_action +=setup_vars;
      *static_lib_action +=static_lib_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::STATIC_LIB), true));
      unique_ptr<generator> g(new static_lib_generator(e, generator_prefix + ".linker.static_lib", source, target, true, static_lib_action, generator_condition));
      e.generators().insert(std::move(g));
   }

   // ... -> SHARED_LIB IMPORT_LIB
   { 
      std::shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      std::shared_ptr<product_argument_writer> import_lib_product(new product_argument_writer("import_lib_product", e.get_type_registry().get(types::IMPORT_LIB)));
      std::shared_ptr<product_argument_writer> shared_lib_rel_product(new product_argument_writer("shared_lib_rel_product", e.get_type_registry().get(types::SHARED_LIB)));
      std::shared_ptr<product_argument_writer> shared_lib_product(new product_argument_writer("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB),
                                                                                              product_argument_writer::output_strategy::FULL_PATH));
      cmdline_builder shared_lib_cmd(config_data.linker_.string() + " \"@$(shared_lib_rel_product).rsp\"");
      
      shared_lib_cmd += shared_lib_product;
      shared_lib_cmd += shared_lib_rel_product;

      cmdline_builder shared_lib_rsp(" /OUT:\"$(shared_lib_product)\" /NOLOGO /DLL $(link_flags) $(user_link_flags) $(searched_lib_searched_dirs) /IMPLIB:\"$(import_lib_product)\" $(obj_sources) $(res_sources) $(static_lib_sources) $(prebuilt_lib_sources) $(searched_lib_sources) $(import_lib_sources)");
      shared_lib_rsp += link_flags;
      shared_lib_rsp += user_link_flags;
      shared_lib_rsp += searched_lib_searched_dirs;
      shared_lib_rsp += obj_sources;
      shared_lib_rsp += res_sources;
      shared_lib_rsp += static_lib_sources;
      shared_lib_rsp += prebuilt_lib_sources;
      shared_lib_rsp += searched_lib_sources;
      shared_lib_rsp += import_lib_sources;
      shared_lib_rsp += import_lib_product;
      shared_lib_rsp += shared_lib_product;

      auto shared_lib_action = std::make_shared<cmdline_action>("link-shared-lib", shared_lib_rel_product, shared_lib_rsp);
      *shared_lib_action += setup_vars;
      *shared_lib_action += shared_lib_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::RES), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::SHARED_LIB), true));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::IMPORT_LIB), true));

      unique_ptr<generator> g(new exe_and_shared_lib_generator(e, generator_prefix + ".linker.shared_lib", source, target, true, shared_lib_action, generator_condition, nullptr));
      e.generators().insert(std::move(g));
   }

   register_configured(version_id, constraints, make_toolset_info(config_data));
}

static
const std::vector<std::pair<string, location_t>>
known_versions_v = {
   {"14.0", R"(c:\Program Files\Microsoft Visual Studio 14.0\VC)"},
   {"12.0", R"(c:\Program Files\Microsoft Visual Studio 12.0\VC)"},
   {"11.0", R"(c:\Program Files\Microsoft Visual Studio 11.0\VC)"}
};

static
const std::map<string, location_t>
known_versions_m(known_versions_v.begin(), known_versions_v.end());

void msvc_toolset::use_toolset_rule(invocation_context& ctx,
                                    const parscore::identifier& version,
                                    const parscore::identifier* path_to_vc_folder) {
   const auto sversion = version.to_string();
   location_t toolset_home;

   engine& e = ctx.current_project_.get_engine();

   if (path_to_vc_folder)
      toolset_home = path_to_vc_folder->to_string();
   else {
      auto i = known_versions_m.find(sversion);
      if (i == known_versions_m.end())
         throw ast2objects_semantic_error(version.start_loc(), "Don't know how to configure this version");

      toolset_home = i->second;
   }

   init_toolset(e, sversion, toolset_home);
}

void msvc_toolset::configure(engine& e,
                             const std::string& version) {
   auto i = known_versions_m.find(version);
   if (i == known_versions_m.end())
      throw std::runtime_error("[msvc_toolset] Don't know how to configure '" + version + "' version");

   init_toolset(e, i->first, i->second);
}

void msvc_toolset::autoconfigure(engine& e) {
   for (const auto& v : known_versions_v) {
      if (exists(v.second))
         init_toolset(e, v.first, v.second);
   }
}

}
