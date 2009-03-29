#include "stdafx.h"
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/cmdline_action.h>
#include <hammer/core/fs_argument_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/free_feature_arg_writer.h>
#include <hammer/core/types.h>
#include <hammer/core/generator.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/exe_and_shared_lib_generator.h>
#include <hammer/core/static_lib_generator.h>

using namespace boost;
using namespace std;

namespace hammer{

gcc_toolset::gcc_toolset() : toolset("gcc")
{

}

gcc_toolset::gcc_install_data 
gcc_toolset::resolve_install_data(const location_t* toolset_home_) const
{
   location_t toolset_home(toolset_home_ == NULL ? location_t() : *toolset_home_);

   gcc_install_data install_data;
   install_data.version_ = "unknown";
   install_data.compiler_ = toolset_home / "g++";
   install_data.linker_ = toolset_home / "g++";
   install_data.librarian_ = toolset_home / "ar";

   return install_data;
}

void gcc_toolset::init_impl(engine& e, const std::string& version_id,
                            const location_t* toolset_home) const
{
   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value("gcc"))
      toolset_def.extend_legal_values("gcc");

   feature_set* generator_condition = e.feature_registry().make_set();
   generator_condition->join("toolset", "gcc");

   gcc_install_data install_data(resolve_install_data(toolset_home));

   toolset_def.get_subfeature("version").extend_legal_values(install_data.version_);

   shared_ptr<product_argument_writer> obj_product(new product_argument_writer("obj_product", e.get_type_registry().get(types::OBJ)));
   shared_ptr<source_argument_writer> static_lib_sources(new source_argument_writer("static_lib_sources", e.get_type_registry().get(types::STATIC_LIB)));
   shared_ptr<source_argument_writer> shared_lib_sources(new source_argument_writer("shared_lib_sources", e.get_type_registry().get(types::SHARED_LIB)));
   shared_ptr<source_argument_writer> searched_lib_sources(new source_argument_writer("searched_lib_sources", e.get_type_registry().get(types::SEARCHED_LIB)));

   shared_ptr<free_feature_arg_writer> searched_lib_searched_dirs(
      new free_feature_arg_writer("searched_lib_searched_dirs", 
                                  e.feature_registry().get_def("search"), 
                                  string("-L \""), 
                                  string("\"")));

   shared_ptr<fs_argument_writer> cflags(new fs_argument_writer("cflags", e.feature_registry()));
   cflags->add("<optimization>speed", "/O3").
           add("<optimization>space", "/Os").
           add("<optimization>off", "-O0").
           add("<inlining>off", "-fno-inline").
           add("<inlining>on", "-Wno-inline").
           add("<inlining>full", "-finline-functions -Wno-inline").
           add("<warnings>on", "-Wall").
           add("<warnings>off", "-w").
           add("<warnings>all", "-Wall -pedantic").
           add("<warnings-as-errors>on", "-Werror").
           add("<debug-symbols>on", "-g").
           add("<profiling>on", "-pg").
           add("<link>shared", "-fPIC");

   shared_ptr<fs_argument_writer> link_flags(new fs_argument_writer("link_flags", e.feature_registry()));
   link_flags->add("<debug-symbols>on", "-g").
               add("<profiling>on", "-pg");

   shared_ptr<free_feature_arg_writer> user_link_flags(new free_feature_arg_writer("user_link_flags", e.feature_registry().get_def("linkflags")));
   shared_ptr<free_feature_arg_writer> user_cxx_flags(new free_feature_arg_writer("user_cxx_flags", e.feature_registry().get_def("cxxflags")));
   shared_ptr<free_feature_arg_writer> user_c_flags(new free_feature_arg_writer("user_c_flags", e.feature_registry().get_def("cflags")));
   shared_ptr<free_feature_arg_writer> user_archive_flags(new free_feature_arg_writer("user_archive_flags", e.feature_registry().get_def("archiveflags")));

   shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry().get_def("include"), "-I\"", "\""));
   shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry().get_def("define"), "-D"));

   // C -> OBJ
   {
      shared_ptr<source_argument_writer> c_input(new source_argument_writer("c_input", e.get_type_registry().get(types::C)));
      cmdline_builder obj_cmd(install_data.compiler_.native_file_string() + 
                              " -x c -c $(cflags) $(user_c_flags) $(includes) $(defines) $(c_input) -o \"$(obj_product)\"");
      obj_cmd += cflags;
      obj_cmd += user_c_flags;
      obj_cmd += c_input;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto_ptr<cmdline_action> obj_action(new cmdline_action("compile-c", obj_product));
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      auto_ptr<generator> g(new generator(e, "gcc.c.compiler", source, target, false, generator_condition));
      g->action(obj_action);
      e.generators().insert(g);
   }

   // CPP -> OBJ
   {
      shared_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP)));
      cmdline_builder obj_cmd(install_data.compiler_.native_file_string() + 
                              " -c -ftemplate-depth-128 $(cflags) $(user_cxx_flags) $(includes) $(defines) $(cpp_input) -o \"$(obj_product)\"");
      obj_cmd += cflags;
      obj_cmd += user_cxx_flags;
      obj_cmd += cpp_input;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto_ptr<cmdline_action> obj_action(new cmdline_action("compile-c++", obj_product));
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      auto_ptr<generator> g(new generator(e, "gcc.cpp.compiler", source, target, false, generator_condition));
      g->action(obj_action);
      e.generators().insert(g);
   }

   // ... -> EXE
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> exe_product(new product_argument_writer("exe_product", e.get_type_registry().get(types::EXE)));
      auto_ptr<cmdline_action> exe_action(new cmdline_action("link-exe", exe_product));
      cmdline_builder exe_cmd(install_data.linker_.native_file_string() + " $(link_flags) $(searched_lib_searched_dirs) -o \"$(exe_product)\" $(obj_sources) $(static_lib_sources) $(searched_lib_sources) $(shared_lib_sources)\n");
      
      exe_cmd += link_flags;
      exe_cmd += searched_lib_searched_dirs;
      exe_cmd += obj_sources;
      exe_cmd += static_lib_sources;
      exe_cmd += searched_lib_sources;
      exe_cmd += shared_lib_sources;
      exe_cmd += exe_product;
      *exe_action += exe_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SHARED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE)));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "gcc.exe.linker", source, target, true, generator_condition));
      g->action(exe_action);
      e.generators().insert(g);
   }

   // ... -> STATIC_LIB
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> static_lib_product(new product_argument_writer("static_lib_product", e.get_type_registry().get(types::STATIC_LIB)));
      cmdline_builder static_lib_cmd(install_data.librarian_.native_file_string() + " $(user_archive_flags) rc $(static_lib_product) $(obj_sources)");
      
      static_lib_cmd += static_lib_product;
      static_lib_cmd += obj_sources;
      static_lib_cmd += user_archive_flags;

      auto_ptr<cmdline_action> static_lib_action(new cmdline_action("link-static-lib", static_lib_product));
      *static_lib_action +=static_lib_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::STATIC_LIB), true));
      auto_ptr<generator> g(new static_lib_generator(e, "gcc.static_lib.linker", source, target, true, generator_condition));
      g->action(static_lib_action);
      e.generators().insert(g);
   }

   // ... -> SHARED_LIB
   { 
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> shared_lib_product(new product_argument_writer("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB)));
      cmdline_builder shared_lib_cmd(install_data.linker_.native_file_string() + " -shared $(link_flags) $(searched_lib_searched_dirs) -o \"$(shared_lib_product)\" $(obj_sources) $(static_lib_sources) $(searched_lib_sources) $(shared_lib_sources)\n");
      shared_lib_cmd += link_flags;
      shared_lib_cmd += searched_lib_searched_dirs;
      shared_lib_cmd += obj_sources;
      shared_lib_cmd += static_lib_sources;
      shared_lib_cmd += searched_lib_sources;
      shared_lib_cmd += shared_lib_sources;
      shared_lib_cmd += shared_lib_product;

      auto_ptr<cmdline_action> shared_lib_action(new cmdline_action("link-shared-lib", shared_lib_product));
      *shared_lib_action += shared_lib_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::SHARED_LIB), true));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::IMPORT_LIB), true));

      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "gcc.shared_lib.linker", source, target, true, generator_condition));
      g->action(shared_lib_action);
      e.generators().insert(g);
   }

}


}
