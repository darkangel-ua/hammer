#include "stdafx.h"
#include <boost/filesystem/operations.hpp>
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
#include <hammer/core/unix_libraries_argument_writer.h>
#include <hammer/core/testing_generators.h>

using namespace boost;
using std::string;
using std::unique_ptr;

namespace hammer{

gcc_toolset::gcc_toolset() : toolset("gcc")
{

}

gcc_toolset::gcc_install_data
gcc_toolset::resolve_install_data(const location_t* toolset_home_, const std::string& version_id) const
{
   location_t toolset_home(toolset_home_ == NULL ? location_t() : *toolset_home_);

   gcc_install_data install_data;
   install_data.version_ = version_id;
   if (version_id.empty() || version_id == "system") {
      install_data.version_ = "system";
      install_data.compiler_ = toolset_home / "g++" ;
      install_data.linker_ = toolset_home / "g++";
   } else {
      install_data.compiler_ = toolset_home / ("g++-" + version_id);
      install_data.linker_ = toolset_home / ("g++-" + version_id);
   } 

   install_data.librarian_ = toolset_home / "ar";
   if (!exists(install_data.librarian_))
      install_data.librarian_ = "ar";

   return install_data;
}

void gcc_toolset::init_impl(engine& e, const std::string& version_id,
                            const location_t* toolset_home) const
{
   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value(name()))
      toolset_def.extend_legal_values(name());

   feature_set* generator_condition = e.feature_registry().make_set();
   generator_condition->join("toolset", name().c_str());

   gcc_install_data install_data(resolve_install_data(toolset_home, version_id));

   toolset_def.get_subfeature("version").extend_legal_values(name(), install_data.version_);

   shared_ptr<product_argument_writer> obj_product(new product_argument_writer("obj_product", e.get_type_registry().get(types::OBJ)));
   shared_ptr<source_argument_writer> static_lib_sources(new source_argument_writer("static_lib_sources", e.get_type_registry().get(types::STATIC_LIB), true, source_argument_writer::FULL_PATH));
   shared_ptr<source_argument_writer> shared_lib_sources(new source_argument_writer("shared_lib_sources", e.get_type_registry().get(types::SHARED_LIB), true, source_argument_writer::FULL_PATH));
   shared_ptr<source_argument_writer> searched_lib_sources(new source_argument_writer("searched_lib_sources", e.get_type_registry().get(types::SEARCHED_STATIC_LIB), true, source_argument_writer::WITHOUT_PATH, "", "-l"));

   shared_ptr<free_feature_arg_writer> searched_lib_searched_dirs(
      new free_feature_arg_writer("searched_lib_searched_dirs",
                                  e.feature_registry().get_def("search"),
                                  string("-L \""),
                                  string("\"")));

   shared_ptr<fs_argument_writer> cflags(new fs_argument_writer("cflags", e.feature_registry()));
   cflags->add("<optimization>speed", "-O3").
           add("<optimization>space", "-Os").
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
           add("<link>shared/<host-os>linux", "-fPIC").
           add("<address-model>32", "-m32").
           add("<address-model>64", "-m64");

   shared_ptr<fs_argument_writer> link_flags(new fs_argument_writer("link_flags", e.feature_registry()));
   link_flags->add("<debug-symbols>on", "-g").
               add("<profiling>on", "-pg").
               add("<address-model>32", "-m32").
               add("<address-model>64", "-m64").
               add("<runtime-link>static", "-static-libgcc -static-libstdc++");

   shared_ptr<free_feature_arg_writer> user_link_flags(new free_feature_arg_writer("user_link_flags", e.feature_registry().get_def("linkflags")));
   shared_ptr<free_feature_arg_writer> user_cxx_flags(new free_feature_arg_writer("user_cxx_flags", e.feature_registry().get_def("cxxflags")));
   shared_ptr<free_feature_arg_writer> user_c_flags(new free_feature_arg_writer("user_c_flags", e.feature_registry().get_def("cflags")));
   shared_ptr<free_feature_arg_writer> user_archive_flags(new free_feature_arg_writer("user_archive_flags", e.feature_registry().get_def("archiveflags")));

   shared_ptr<free_feature_arg_writer> includes(new free_feature_arg_writer("includes", e.feature_registry().get_def("include"), "-I\"", "\""));
   shared_ptr<free_feature_arg_writer> defines(new free_feature_arg_writer("defines", e.feature_registry().get_def("define"), "-D"));

   const string generator_prefix = name() + "-" + install_data.version_;

   // C -> OBJ
   {
      shared_ptr<source_argument_writer> c_input(new source_argument_writer("c_input", e.get_type_registry().get(types::C), /*exact_type=*/false, source_argument_writer::FULL_PATH));
      cmdline_builder obj_cmd(install_data.compiler_.string() +
                              " -x c -c $(cflags) $(user_c_flags) $(includes) $(defines) -o \"$(obj_product)\" $(c_input)");
      obj_cmd += cflags;
      obj_cmd += user_c_flags;
      obj_cmd += c_input;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      unique_ptr<cmdline_action> obj_action(new cmdline_action("compile-c", obj_product));
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      unique_ptr<generator> g(new generator(e, generator_prefix + ".compiler.c", source, target, false, generator_condition));

      std::unique_ptr<generator> g_copy(new generator(e, generator_prefix + ".compiler.c", source, target, false, generator_condition));
      std::unique_ptr<cmdline_action> obj_action_copy(new cmdline_action(*obj_action));

      g->action(std::move(obj_action));

      add_compile_fail_generator(e, std::move(g_copy), std::move(obj_action_copy));
      e.generators().insert(std::move(g));
   }

   // CPP -> OBJ
   {
      shared_ptr<fs_argument_writer> cxxflags(new fs_argument_writer("cxxflags", e.feature_registry()));
      cxxflags->add("<rtti>off", "-fno-rtti");

      shared_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP), /*exact_type=*/false, source_argument_writer::FULL_PATH));
      cmdline_builder obj_cmd(install_data.compiler_.string() +
                              " -c -ftemplate-depth-128 $(cflags) $(cxxflags) $(user_cxx_flags) $(includes) $(defines) -o \"$(obj_product)\" $(cpp_input)");
      obj_cmd += cflags;
      obj_cmd += cxxflags;
      obj_cmd += user_cxx_flags;
      obj_cmd += cpp_input;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      unique_ptr<cmdline_action> obj_action(new cmdline_action("compile-c++", obj_product));
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));
      unique_ptr<generator> g(new generator(e, generator_prefix + ".compiler.cpp", source, target, false, generator_condition));

      std::unique_ptr<generator> g_copy(new generator(e, generator_prefix + ".compiler.cpp", source, target, false, generator_condition));
      std::unique_ptr<cmdline_action> obj_action_copy(new cmdline_action(*obj_action));

      g->action(std::move(obj_action));

      add_compile_fail_generator(e, std::move(g_copy), std::move(obj_action_copy));
      e.generators().insert(std::move(g));
   }

   // ... -> SHARED_LIB
   {
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> shared_lib_product(new product_argument_writer("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB)));
      shared_ptr<unix_libraries_argument_writer> libraries_writer(new unix_libraries_argument_writer("libraries", linker_type::GNU, e));
      cmdline_builder shared_lib_cmd(install_data.linker_.string() + " -shared $(link_flags) $(searched_lib_searched_dirs) -o \"$(shared_lib_product)\" $(obj_sources) $(libraries)\n");
      shared_lib_cmd += link_flags;
      shared_lib_cmd += searched_lib_searched_dirs;
      shared_lib_cmd += obj_sources;
      shared_lib_cmd += libraries_writer;
      shared_lib_cmd += shared_lib_product;

      unique_ptr<cmdline_action> shared_lib_action(new cmdline_action("link-shared-lib", shared_lib_product));
      *shared_lib_action += shared_lib_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SHARED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_SHARED_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::SHARED_LIB), true));

      unique_ptr<generator> g(new exe_and_shared_lib_generator(e, generator_prefix + ".linker.shared_lib", source, target, true, generator_condition));
      g->action(std::move(shared_lib_action));
      e.generators().insert(std::move(g));
   }

   // ... -> EXE
   {
      shared_ptr<free_feature_arg_writer> ld_library_path_dirs(
         new free_feature_arg_writer("ld_library_path_dirs",
                                     e.feature_registry().get_def("search"),
                                     string(),
                                     string(),
                                     ":"));

      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> exe_product(new product_argument_writer("exe_product", e.get_type_registry().get(types::EXE)));
      shared_ptr<unix_libraries_argument_writer> libraries_writer(new unix_libraries_argument_writer("libraries", linker_type::GNU, e));
      unique_ptr<cmdline_action> exe_action(new cmdline_action("link-exe", exe_product));
      cmdline_builder exe_cmd("LD_LIBRARY_PATH=$(ld_library_path_dirs):LD_LIBRARY_PATH " + install_data.linker_.string() + " $(link_flags) $(searched_lib_searched_dirs) -o \"$(exe_product)\" $(obj_sources) $(libraries)\n");

      exe_cmd += link_flags;
      exe_cmd += searched_lib_searched_dirs;
      exe_cmd += ld_library_path_dirs;
      exe_cmd += obj_sources;
      exe_cmd += libraries_writer;
      exe_cmd += exe_product;
      *exe_action += exe_cmd;

      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SHARED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_SHARED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE)));
      unique_ptr<generator> g(new exe_and_shared_lib_generator(e, generator_prefix + ".linker.exe", source, target, true, generator_condition));
      g->action(std::move(exe_action));
      e.generators().insert(std::move(g));
   }

   // ... -> STATIC_LIB
   {
      shared_ptr<source_argument_writer> obj_sources(new source_argument_writer("obj_sources", e.get_type_registry().get(types::OBJ)));
      shared_ptr<product_argument_writer> static_lib_product(new product_argument_writer("static_lib_product", e.get_type_registry().get(types::STATIC_LIB)));
      cmdline_builder static_lib_cmd(install_data.librarian_.string() + " $(user_archive_flags) rc $(static_lib_product) $(obj_sources)");

      static_lib_cmd += static_lib_product;
      static_lib_cmd += obj_sources;
      static_lib_cmd += user_archive_flags;

      unique_ptr<cmdline_action> static_lib_action(new cmdline_action("link-static-lib", static_lib_product));
      *static_lib_action +=static_lib_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SHARED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_SHARED_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::STATIC_LIB), true));
      unique_ptr<generator> g(new static_lib_generator(e, generator_prefix + ".linker.static_lib", source, target, true, generator_condition));
      g->action(std::move(static_lib_action));
      e.generators().insert(std::move(g));
   }
}

void gcc_toolset::autoconfigure(engine& e) const
{
   init_impl(e, "system", NULL);
}

}
