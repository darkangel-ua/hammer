#include "stdafx.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/assign/list_of.hpp>
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

using namespace boost::assign;
using std::string;
using std::unique_ptr;
using namespace boost;

namespace hammer{

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

struct msvc_toolset::impl_t
{
   impl_t();
};

msvc_toolset::impl_t::impl_t()
{

}

msvc_toolset::msvc_toolset() : toolset("msvc"), impl_(new impl_t)
{

};

msvc_toolset::~msvc_toolset()
{
   delete impl_;
}

void msvc_toolset::init_impl(engine& e,
                             const std::string& version_id,
                             const location_t* toolset_home) const
{
   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value("msvc"))
      toolset_def.extend_legal_values("msvc");

   if (!toolset_home)
      throw std::runtime_error("[msvc_toolset]: You must specify toolset home directory");

   if (!version_id.empty())
      toolset_def.get_subfeature("version").extend_legal_values(version_id);
   init(e, version_id, *toolset_home);
}

void msvc_toolset::init(engine& e,
                        const std::string& version_id,
                        const location_t& toolset_home) const
{
   msvc_data config_data;
   config_data.setup_script_ = toolset_home / "bin/vcvars32.bat";
   config_data.compiler_ = "cl.exe";
   config_data.librarian_ = "lib.exe";
   config_data.linker_ = "link.exe";
   config_data.manifest_tool_ = "mt.exe";
   config_data.resource_compiler_ = "rc.exe";

   {
      feature_attributes fa = {0};
      fa.propagated = true;
      feature_def debug_store("debug-store", list_of("database")("object"), fa);
      e.feature_registry().add_def(debug_store);
   }

   feature_set* generator_condition = e.feature_registry().make_set();
   if (!version_id.empty())
      generator_condition->join("toolset", ("msvc-" + version_id).c_str());
   else
      generator_condition->join("toolset", "msvc");

   cmdline_builder setup_vars("call \"" + config_data.setup_script_.string() + "\" >nul");
   shared_ptr<source_argument_writer> static_lib_sources(new source_argument_writer("static_lib_sources", e.get_type_registry().get(types::STATIC_LIB), true, source_argument_writer::FULL_PATH));
   shared_ptr<source_argument_writer> searched_lib_sources(new source_argument_writer("searched_lib_sources", e.get_type_registry().get(types::SEARCHED_LIB), false, source_argument_writer::WITHOUT_PATH));
   shared_ptr<source_argument_writer> prebuilt_lib_sources(new source_argument_writer("prebuilt_lib_sources", e.get_type_registry().get(types::PREBUILT_STATIC_LIB), true, source_argument_writer::FULL_PATH));
   shared_ptr<source_argument_writer> import_lib_sources(new source_argument_writer("import_lib_sources", e.get_type_registry().get(types::IMPORT_LIB), true, source_argument_writer::FULL_PATH));

   shared_ptr<product_argument_writer> obj_product(new product_argument_writer("obj_product", e.get_type_registry().get(types::OBJ)));
   shared_ptr<product_argument_writer> pch_product(new product_argument_writer("pch_product", e.get_type_registry().get(types::PCH)));

   shared_ptr<free_feature_arg_writer> searched_lib_searched_dirs(
      new free_feature_arg_writer("searched_lib_searched_dirs", 
                                  e.feature_registry().get_def("search"), 
                                  string(), 
                                  string(), 
                                  ";",
                                  "/LIBPATH:\"",
                                  "\""));

   shared_ptr<fs_argument_writer> link_flags(new fs_argument_writer("link_flags", e.feature_registry()));
   link_flags->add("<debug-symbols>on", "/DEBUG /INCREMENTAL").
               add("<debug-symbols>on/<runtime-debugging>off", "/OPT:REF,ICF").
               add("<user-interface>console", "/subsystem:console").
               add("<user-interface>gui", "/subsystem:windows").
               add("<user-interface>gui/<character-set>unicode", "/ENTRY:\"wWinMainCRTStartup\"").
               add("<user-interface>wince", "/subsystem:windowsce").
               add("<user-interface>native", "/subsystem:native").
               add("<user-interface>auto", "/subsystem:posix");

   shared_ptr<fs_argument_writer> cflags(new fs_argument_writer("cflags", e.feature_registry()));
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

   shared_ptr<fs_argument_writer> cppflags(new fs_argument_writer("cppflags", e.feature_registry()));
   cppflags->add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>off", "/EHs").
             add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>on", "/EHsc").
             add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>off", "/EHa").
             add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>on", "/EHac");

   shared_ptr<free_feature_arg_writer> user_c_flags(new free_feature_arg_writer("user_c_flags", e.feature_registry().get_def("cflags")));
   shared_ptr<free_feature_arg_writer> user_link_flags(new free_feature_arg_writer("user_link_flags", e.feature_registry().get_def("linkflags")));
   shared_ptr<free_feature_arg_writer> user_cxx_flags(new free_feature_arg_writer("user_cxx_flags", e.feature_registry().get_def("cxxflags")));
   shared_ptr<free_feature_arg_writer> user_archive_flags(new free_feature_arg_writer("user_archive_flags", e.feature_registry().get_def("archiveflags")));

   shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry().get_def("include"), "-I \"", "\""));
   shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry().get_def("define"), "-D \"", "\""));
   shared_ptr<free_feature_arg_writer> undefines(new free_feature_arg_writer("undefines", e.feature_registry().get_def("undef"), "-U \"", "\""));

   shared_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP), /*exact_type=*/false));
   shared_ptr<source_argument_writer> res_sources(new source_argument_writer("res_sources", e.get_type_registry().get(types::RES)));
   shared_ptr<pch_argument_writer> create_pch_header(new pch_argument_writer("create_pch_header", pch_argument_writer::part::header,
                                                                             "/Yc\"", "\""));
   shared_ptr<pch_argument_writer> use_pch_header(new pch_argument_writer("use_pch_header", pch_argument_writer::part::header,
                                                                          "/Yu\"", "\""));
   shared_ptr<pch_argument_writer> use_pch_product(new pch_argument_writer("use_pch_product", pch_argument_writer::part::product,
                                                                           "/Fp\"", "\""));
   shared_ptr<output_dir_argument_writer> output_dir(new output_dir_argument_writer("output_dir"));

   // CPP -> OBJ
   {
      cmdline_builder obj_cmd(config_data.compiler_.string() +
                              " /c /nologo $(cflags) $(cppflags) $(user_cxx_flags) $(includes) $(undefines) $(defines) $(use_pch_header) $(use_pch_product) $(cpp_input)"
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

      unique_ptr<cmdline_action> obj_action(new cmdline_action("compile-c++", obj_product));
      *obj_action += setup_vars;
      *obj_action += obj_cmd;

      boost::shared_ptr<batched_cmdline_action> batched_obj_action(new batched_cmdline_action("batched compile-c++"));
      *batched_obj_action += setup_vars;
      *batched_obj_action += batched_obj_cmd;

//      obj_action->batched_action(batched_obj_action);

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      unique_ptr<generator> g(new generator(e, "msvc.cpp.compiler", source, target, false, generator_condition));
      g->action(std::move(obj_action));
      e.generators().insert(std::move(g));
   }
   
   // CPP + H -> PCH + OBJ
   {
      cmdline_builder obj_cmd(config_data.compiler_.string() +
                              " /c /nologo $(create_pch_header) $(cflags) $(cppflags) $(user_cxx_flags) $(includes) $(undefines) $(defines) $(cpp_input)"
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
      unique_ptr<cmdline_action> obj_action(new cmdline_action("compile-c++-pch", obj_product));
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
      unique_ptr<generator> g(new pch_generator(e, "msvc.c++-pch.compiler", source, target, true, constraints));
      g->action(std::move(obj_action));
      e.generators().insert(std::move(g));
   }

   // C -> OBJ
   {
      shared_ptr<source_argument_writer> c_source(new source_argument_writer("c_source", e.get_type_registry().get(types::C)));
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

      unique_ptr<cmdline_action> obj_action(new cmdline_action("compile-c", obj_product));
      *obj_action += setup_vars;
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      unique_ptr<generator> g(new generator(e, "msvc.c.compiler", source, target, false, generator_condition));
      g->action(std::move(obj_action));
      e.generators().insert(std::move(g));
   }

   // RC -> RES
   {
      shared_ptr<source_argument_writer> rc_source(new source_argument_writer("rc_source", e.get_type_registry().get(types::RC)));
      shared_ptr<product_argument_writer> res_product(new product_argument_writer("res_product", e.get_type_registry().get(types::RES)));
      cmdline_builder res_cmd(config_data.resource_compiler_.string() + " $(includes) $(undefines) $(defines) /Fo\"$(res_product)\" $(rc_source)");

      res_cmd += rc_source;
      res_cmd += includes;
      res_cmd += undefines;
      res_cmd += defines;
      res_cmd += res_product;

      unique_ptr<cmdline_action> res_action(new cmdline_action("compile-rc", res_product));
      *res_action += setup_vars;
      *res_action += res_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::RC), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::RES)));
      unique_ptr<generator> g(new generator(e, "msvc.resource-compiler", source, target, false, generator_condition));
      g->action(std::move(res_action));
      e.generators().insert(std::move(g));
   }

   // ... -> EXE
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> exe_product_unc(new product_argument_writer("exe_product_unc", e.get_type_registry().get(types::EXE),
                                                                                      product_argument_writer::output_strategy::FULL_PATH));
      shared_ptr<product_argument_writer> exe_product(new product_argument_writer("exe_product", e.get_type_registry().get(types::EXE)));
      shared_ptr<product_argument_writer> exe_manifest_product(new product_argument_writer("exe_manifest_product", e.get_type_registry().get(types::EXE_MANIFEST)));
      cmdline_builder exe_cmd(config_data.linker_.string() + " \"@$(exe_product).rsp\"\n"
                              "if %ERRORLEVEL% NEQ 0 EXIT %ERRORLEVEL%\n"
                              "if exist \"$(exe_manifest_product)\" (" + config_data.manifest_tool_.string() + " -nologo -manifest \"$(exe_manifest_product)\" \"-outputresource:$(exe_product);1\")");

      exe_cmd += exe_product;
      exe_cmd += exe_manifest_product;
      cmdline_builder exe_rsp("  /nologo /MANIFEST /MANIFESTFILE:$(exe_manifest_product) $(link_flags) $(user_link_flags) $(searched_lib_searched_dirs) /out:\"$(exe_product_unc)\" $(obj_sources) $(res_sources) $(static_lib_sources) $(prebuilt_lib_sources) $(searched_lib_sources) $(import_lib_sources)");

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
      exe_rsp += exe_manifest_product;

      unique_ptr<cmdline_action> exe_action(new cmdline_action("link-exe", exe_product, exe_rsp));
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
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE_MANIFEST)));
      unique_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.exe.linker", source, target, true, generator_condition));
      g->action(std::move(exe_action));
      e.generators().insert(std::move(g));
   }

   // ... -> STATIC_LIB
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> static_lib_product(new product_argument_writer("static_lib_product", e.get_type_registry().get(types::STATIC_LIB)));
      shared_ptr<product_argument_writer> static_lib_product_unc(new product_argument_writer("static_lib_product_unc", 
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
      unique_ptr<cmdline_action> static_lib_action(new cmdline_action("link-static-lib", static_lib_product, static_lib_rsp));
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
      unique_ptr<generator> g(new static_lib_generator(e, "msvc.static_lib.linker", source, target, true, generator_condition));
      g->action(std::move(static_lib_action));
      e.generators().insert(std::move(g));
   }

   // ... -> SHARED_LIB IMPORT_LIB
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> import_lib_product(new product_argument_writer("import_lib_product", e.get_type_registry().get(types::IMPORT_LIB)));
      shared_ptr<product_argument_writer> shared_lib_rel_product(new product_argument_writer("shared_lib_rel_product", e.get_type_registry().get(types::SHARED_LIB)));
      shared_ptr<product_argument_writer> shared_lib_product(new product_argument_writer("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB), 
                                                                                         product_argument_writer::output_strategy::FULL_PATH));
      shared_ptr<product_argument_writer> dll_manifest_product(new product_argument_writer("dll_manifest_product", e.get_type_registry().get(types::DLL_MANIFEST)));
      cmdline_builder shared_lib_cmd(config_data.linker_.string() + " \"@$(shared_lib_rel_product).rsp\"\n"
                                     "if %ERRORLEVEL% NEQ 0 EXIT %ERRORLEVEL%\n"
                                     "if exist \"$(dll_manifest_product)\" (" + config_data.manifest_tool_.string() + " -nologo -manifest \"$(dll_manifest_product)\" \"-outputresource:$(shared_lib_rel_product);2\")");
      
      shared_lib_cmd += shared_lib_product;
      shared_lib_cmd += shared_lib_rel_product;
      shared_lib_cmd += dll_manifest_product;

      cmdline_builder shared_lib_rsp(" /OUT:\"$(shared_lib_product)\" /NOLOGO /DLL /MANIFEST /MANIFESTFILE:$(dll_manifest_product) $(link_flags) $(user_link_flags) $(searched_lib_searched_dirs) /IMPLIB:\"$(import_lib_product)\" $(obj_sources) $(res_sources) $(static_lib_sources) $(prebuilt_lib_sources) $(searched_lib_sources) $(import_lib_sources)");
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
      shared_lib_rsp += dll_manifest_product;

      unique_ptr<cmdline_action> shared_lib_action(new cmdline_action("link-shared-lib", shared_lib_rel_product, shared_lib_rsp));
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
      target.push_back(generator::produced_type(e.get_type_registry().get(types::DLL_MANIFEST), true));

      unique_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.shared_lib.linker", source, target, true, generator_condition));
      g->action(std::move(shared_lib_action));
      e.generators().insert(std::move(g));
   }
}

void msvc_toolset::autoconfigure(engine& e) const
{
   using namespace std;
   const vector<pair<string, location_t> > known_versions =
      {
         {"11.0", R"(c:\Program Files\Microsoft Visual Studio 11.0\VC)"},
         {"12.0", R"(c:\Program Files\Microsoft Visual Studio 12.0\VC)"},
         {"14.0", R"(c:\Program Files\Microsoft Visual Studio 14.0\VC)"},
      };

   for (const auto& v : known_versions) {
      if (exists(v.second))
         init_impl(e, v.first, &v.second);
   }
}

}
