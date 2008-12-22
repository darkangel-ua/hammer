#include "stdafx.h"
#include "msvc_generator.h"
#include <boost/assign/list_of.hpp>
#include "generator.h"
#include "types.h"
#include "engine.h"
#include "type_registry.h"
#include "generator_registry.h"
#include "static_lib_generator.h"
#include "exe_and_shared_lib_generator.h"
#include "header_lib_generator.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "pch_generator.h"
#include "cmdline_action.h"
#include "fs_argument_writer.h"
#include "source_argument_writer.h"
#include "product_argument_writer.h"
#include "free_feature_arg_writer.h"

using namespace boost::assign;
using namespace std;
using namespace boost;

namespace hammer{

void add_msvc_generators(engine& e, generator_registry& gr)
{
   e.feature_registry().get_def("toolset").extend_legal_values("msvc");
   
   {
      feature_attributes fa;
      fa.propagated = true;
      feature_def debug_store("debug-store", list_of("object")("database"), fa);
      e.feature_registry().add_def(debug_store);
   }

   cmdline_builder setup_vars("call \"C:\\Program Files\\Microsoft Visual Studio 8\\VC\\bin\\vcvars32.bat\" >nul");
   shared_ptr<source_argument_writer> static_lib_sources(new source_argument_writer("static_lib_sources", e.get_type_registry().get(types::STATIC_LIB)));
   shared_ptr<source_argument_writer> searched_lib_sources(new source_argument_writer("searched_lib_sources", e.get_type_registry().get(types::SEARCHED_LIB)));
   shared_ptr<source_argument_writer> import_lib_sources(new source_argument_writer("import_lib_sources", e.get_type_registry().get(types::IMPORT_LIB)));

   shared_ptr<product_argument_writer> obj_product(new product_argument_writer("obj_product", e.get_type_registry().get(types::OBJ)));

   shared_ptr<free_feature_arg_writer> searched_lib_searched_dirs(
      new free_feature_arg_writer("searched_lib_searched_dirs", 
                                  e.feature_registry().get_def("search"), 
                                  string(), 
                                  string(), 
                                  ";",
                                  "/LIBPATH:\"",
                                  "\""));

   shared_ptr<fs_argument_writer> link_flags(new fs_argument_writer("link_flags", e.feature_registry()));
   link_flags->add("<debug-symbols>on", "/DEBUG").
               add("<debug-symbols>on/<runtime-debugging>off", "/OPT:REF,ICF").
               add("<user-interface>console", "/subsystem:console").
               add("<user-interface>gui", "/subsystem:windows").
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
           add("<runtime-debugging>off/<runtime-link>shared", "/MD").
           add("<runtime-debugging>on/<runtime-link>shared", "/MDd").
           add("<runtime-debugging>off/<runtime-link>static/<threading>multi", "/MT").
           add("<runtime-debugging>on/<runtime-link>static/<threading>multi", "/MTd");

   shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry().get_def("include"), "-I \"", "\""));
   shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry().get_def("define"), "-D \"", "\""));
   shared_ptr<free_feature_arg_writer> undefines(new free_feature_arg_writer("undefines", e.feature_registry().get_def("undef"), "-U \"", "\""));

   // CPP -> OBJ
   {
      shared_ptr<fs_argument_writer> cppflags(new fs_argument_writer("cppflags", e.feature_registry()));
      cppflags->add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>off", "/EHs").
                add("<exception-handling>on/<asynch-exceptions>off/<extern-c-nothrow>on", "/EHsc").
                add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>off", "/EHa").
                add("<exception-handling>on/<asynch-exceptions>on/<extern-c-nothrow>on", "/EHac");

      shared_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP)));
      cmdline_builder obj_cmd("cl.exe /c /nologo $(cflags) $(cppflags) $(includes) $(undefines) $(defines) \"$(cpp_input)\" -Fo\"$(obj_product)\"");
      obj_cmd += cflags;
      obj_cmd += cppflags;
      obj_cmd += cpp_input;
      obj_cmd += includes;
      obj_cmd += undefines;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto_ptr<cmdline_action> obj_action(new cmdline_action("compile-c-c++", obj_product));
      *obj_action += setup_vars;
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.cpp.compiler", source, target, false));
      g->action(obj_action);
      e.generators().insert(g);
   }
   
   // CPP + H -> PCH + OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::PCH), 1));

      feature_set* constraints = e.feature_registry().make_set();
      constraints->join("__pch", "");
      auto_ptr<generator> g(new pch_generator(e, "msvc.cpp-pch.compiler", source, target, true, constraints));
      
      e.generators().insert(g);
   }

   // C -> OBJ
   {
      shared_ptr<source_argument_writer> c_source(new source_argument_writer("c_source", e.get_type_registry().get(types::C)));
      cmdline_builder obj_cmd("cl.exe /c /TC /nologo $(cflags) $(includes) $(undefines) $(defines) \"$(c_source)\" -Fo\"$(obj_product)\"");
      obj_cmd += cflags;
      obj_cmd += c_source;
      obj_cmd += includes;
      obj_cmd += undefines;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto_ptr<cmdline_action> obj_action(new cmdline_action("compile-c-c", obj_product));
      *obj_action += setup_vars;
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.c.compiler", source, target, false));
      g->action(obj_action);
      e.generators().insert(g);
   }

   // C + H -> PCH + OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::PCH), 1));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));

      feature_set* constraints = e.feature_registry().make_set();
      constraints->join("__pch", "");
      auto_ptr<generator> g(new pch_generator(e, "msvc.c-pch.compiler", source, target, true, constraints));
      
      e.generators().insert(g);
   }

   // this is generator for obj meta target. Consume all input and transfer only obj types to result products
   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "obj meta target generator", source, target, true));
      e.generators().insert(g);
   }

   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> exe_product(new product_argument_writer("exe_product", e.get_type_registry().get(types::EXE)));
      shared_ptr<product_argument_writer> exe_manifest_product(new product_argument_writer("exe_manifest_product", e.get_type_registry().get(types::EXE_MANIFEST)));
      auto_ptr<cmdline_action> exe_action(new cmdline_action("link-exe", exe_product));
      cmdline_builder exe_cmd("link.exe /nologo $(link_flags) $(searched_lib_searched_dirs) /out:\"$(exe_product)\" $(obj_sources) $(static_lib_sources) $(searched_lib_sources) $(import_lib_sources)\n"
                              "if %ERRORLEVEL% NEQ 0 EXIT %ERRORLEVEL%\n"
                              "if exist \"$(exe_manifest_product)\" (mt -nologo -manifest \"$(exe_manifest_product)\" \"-outputresource:$(exe_product)\")");
      exe_cmd += link_flags;
      exe_cmd += searched_lib_searched_dirs;
      exe_cmd += obj_sources;
      exe_cmd += static_lib_sources;
      exe_cmd += searched_lib_sources;
      exe_cmd += import_lib_sources;
      exe_cmd += exe_product;
      exe_cmd += exe_manifest_product;
      *exe_action += setup_vars;
      *exe_action += exe_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE), 1));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE_MANIFEST), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.exe.linker", source, target, true));
      g->action(exe_action);
      e.generators().insert(g);
   }

   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> static_lib_product(new product_argument_writer("static_lib_product", e.get_type_registry().get(types::STATIC_LIB)));
      cmdline_builder static_lib_rsp("/out:\"$(static_lib_product)\"\n$(obj_sources)");
      static_lib_rsp += obj_sources;
      static_lib_rsp += static_lib_product;
      cmdline_builder static_lib_cmd("if exist \"$(static_lib_product)\" DEL \"$(static_lib_product)\"\n"
                                     "lib.exe /nologo \"@$(static_lib_product).rsp\"");
      
      static_lib_cmd += static_lib_product;
      static_lib_cmd += obj_sources;
      auto_ptr<cmdline_action> static_lib_action(new cmdline_action("link-static-lib", static_lib_product, static_lib_rsp));
      *static_lib_action +=setup_vars;
      *static_lib_action +=static_lib_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::STATIC_LIB), 1));
      auto_ptr<generator> g(new static_lib_generator(e, "msvc.static_lib.linker", source, target, true));
      g->action(static_lib_action);
      e.generators().insert(g);
   }

   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> import_lib_product(new product_argument_writer("import_lib_product", e.get_type_registry().get(types::IMPORT_LIB)));
      shared_ptr<product_argument_writer> shared_lib_product(new product_argument_writer("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB)));
      shared_ptr<product_argument_writer> dll_manifest_product(new product_argument_writer("dll_manifest_product", e.get_type_registry().get(types::DLL_MANIFEST)));
      cmdline_builder shared_lib_cmd("link.exe /DLL /nologo $(link_flags) $(searched_lib_searched_dirs) /out:\"$(shared_lib_product)\" /IMPLIB:\"$(import_lib_product)\" $(obj_sources) $(static_lib_sources) $(searched_lib_sources) $(import_lib_sources)\n"
                                     "if %ERRORLEVEL% NEQ 0 EXIT %ERRORLEVEL%\n"
                                     "if exist \"$(dll_manifest_product)\" (mt -nologo -manifest \"$(dll_manifest_product)\" \"-outputresource:$(shared_lib_product)\")");
      shared_lib_cmd += link_flags;
      shared_lib_cmd += searched_lib_searched_dirs;
      shared_lib_cmd += obj_sources;
      shared_lib_cmd += static_lib_sources;
      shared_lib_cmd += searched_lib_sources;
      shared_lib_cmd += import_lib_sources;
      shared_lib_cmd += import_lib_product;
      shared_lib_cmd += shared_lib_product;
      shared_lib_cmd += dll_manifest_product;

      auto_ptr<cmdline_action> shared_lib_action(new cmdline_action("link-shared-lib", shared_lib_product));
      *shared_lib_action += setup_vars;
      *shared_lib_action += shared_lib_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::SHARED_LIB), 1));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::IMPORT_LIB), 1));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::DLL_MANIFEST), 1));

      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.shared_lib.linker", source, target, true));
      g->action(shared_lib_action);
      e.generators().insert(g);
   }

   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::HEADER_LIB), 1));
      auto_ptr<generator> g(new header_lib_generator(e, "header_lib.linker", source, target));
      e.generators().insert(g);
   }
}

}