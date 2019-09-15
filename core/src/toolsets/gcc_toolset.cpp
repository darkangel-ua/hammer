#include <regex>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_unique.hpp>
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
#include <hammer/core/rule_manager.h>
#include <hammer/core/ast2objects.h>

namespace fs = boost::filesystem;

namespace hammer {

namespace {

class gcc_generator : public generator {
   public:
      gcc_generator(engine& e,
                    const std::string& name,
                    const consumable_types_t& source_types,
                    const producable_types_t& target_types,
                    bool composite,
                    const build_action_ptr& action,
                    const feature_set* constraints,
                    const feature_set* additional_target_properties)
         : generator(e, name, source_types, target_types, composite, action, constraints),
           additional_target_properties_(additional_target_properties)
      {}

   private:
      const feature_set* additional_target_properties_;

      basic_build_target*
      create_target(const main_target* mt,
                    const build_node::sources_t& sources,
                    const std::string* composite_target_name,
                    const produced_type& type,
                    const feature_set* target_properties) const override {
         if (additional_target_properties_) {
            feature_set* new_target_properties = target_properties->clone();
            new_target_properties->join(*additional_target_properties_);
            return generator::create_target(mt, sources, composite_target_name, type, new_target_properties);
         } else
            return generator::create_target(mt, sources, composite_target_name, type, target_properties);
      }
};

}

struct gcc_toolset::toolset_data {
   std::string version_;
   std::string version_suffix_;
   fs::path compiler_;
   fs::path linker_;
   fs::path librarian_;
   feature_set* c_flags_;
   feature_set* cxx_flags_;
   feature_set* link_flags_;
   const feature_set* constraints_;
};

gcc_toolset::gcc_toolset()
   : toolset("gcc",
             rule_manager::make_rule_declaration("use-toolset-gcc",
                                                 this, &gcc_toolset::use_toolset_rule,
                                                 {"version", "c++-compiler", "linker", "librarian", "c-flags", "cxx-flags", "link-flags", "constraints"}))
{
}

YAML::Node
gcc_toolset::make_toolset_info(const toolset_data& td) {
   YAML::Node info;
   auto fs2str = [](const feature_set* fs) {
      if (!fs || fs->empty())
         return YAML::Node();

      return YAML::Node(boost::replace_all_copy(dump_for_hash(*fs, true), "\n", " "));
   };

   info["constraints"] = fs2str(td.constraints_);
   info["cxx-compiler"] = td.compiler_.string();
   info["cxx-flags"] = fs2str(td.cxx_flags_);
   info["c-flags"] = fs2str(td.c_flags_);
   info["linker"] = td.linker_.string();
   info["link-flags"] = fs2str(td.link_flags_);
   info["librarian"] = td.librarian_.string();

   return info;
}

void gcc_toolset::init_toolset(engine& e,
                               const toolset_data& td) {
   if (is_already_configured(td.version_, *td.constraints_))
      throw std::runtime_error("Version '" + td.version_ + "' already registered with similar constraints");

   feature_def& toolset_def = e.feature_registry().get_def("toolset");
   if (!toolset_def.is_legal_value(name()))
      toolset_def.extend_legal_values(name(), e.feature_registry().get_or_create_feature_value_ns("c/c++"));

   if (!toolset_def.get_subfeature("version").is_legal_value(name(), td.version_))
      toolset_def.get_subfeature("version").extend_legal_values(name(), td.version_);

   feature_set* generator_condition = e.feature_registry().make_set();
   generator_condition->join("toolset", (name() + "-" + td.version_).c_str());
   generator_condition->join(*td.constraints_);

   auto obj_product = std::make_shared<product_argument_writer>("obj_product", e.get_type_registry().get(types::OBJ));
   auto searched_lib_searched_dirs = std::make_shared<free_feature_arg_writer>("searched_lib_searched_dirs", e.feature_registry(), "search", "-L \"", "\"");

   auto common_compiler_flags = std::make_shared<fs_argument_writer>("common-compiler-flags", e.feature_registry());
   common_compiler_flags
       ->add("<optimization>speed", "-O3").
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
         add("<pic>on", "-fPIC").
         add("<address-model>32", "-m32").
         add("<address-model>64", "-m64");

   auto link_flags = std::make_shared<fs_argument_writer>("link_flags", e.feature_registry());
   link_flags->add("<debug-symbols>on", "-g").
               add("<profiling>on", "-pg").
               add("<address-model>32", "-m32").
               add("<address-model>64", "-m64").
               add("<runtime-link>static", "-static-libgcc -static-libstdc++");

   auto user_linkflags = std::make_shared<free_feature_arg_writer>("user_linkflags", e.feature_registry(), "linkflags");
   auto user_cxx_flags = std::make_shared<free_feature_arg_writer>("user_cxx_flags", e.feature_registry(), "cxxflags");
   auto user_c_flags = std::make_shared<free_feature_arg_writer>("user_c_flags", e.feature_registry(), "cflags");
   auto user_archive_flags = std::make_shared<free_feature_arg_writer>("user_archive_flags", e.feature_registry(), "archiveflags");

   auto includes = std::make_shared<free_feature_arg_writer>("includes", e.feature_registry(), "include", "-I\"", "\"");
   auto generated_includes = std::make_shared<free_feature_arg_writer>("generated-includes", e.feature_registry(), "__generated-include", "-I\"", "\"");
   auto defines = std::make_shared<free_feature_arg_writer>("defines", e.feature_registry(), "define", "-D");

   const std::string generator_prefix = name() + "-" + td.version_;

   // C -> OBJ
   {
      auto c_input = std::make_shared<source_argument_writer>("c_input", e.get_type_registry().get(types::C), /*exact_type=*/false, source_argument_writer::FULL_PATH);
      cmdline_builder obj_cmd(td.compiler_.string() +
                              " -x c -c $(common-compiler-flags) $(user_c_flags) $(generated-includes) $(includes) $(defines) -o \"$(obj_product)\" $(c_input)");
      obj_cmd += common_compiler_flags;
      obj_cmd += user_c_flags;
      obj_cmd += c_input;
      obj_cmd += generated_includes;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto obj_action = std::make_shared<cmdline_action>("compile-c", obj_product);
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));

      add_compile_generators(e, obj_action, [&](const build_action_ptr& compile_action) {
         return boost::make_unique<gcc_generator>(e, generator_prefix + ".compiler.c", source, target, false, compile_action, generator_condition, td.c_flags_);
      });
   }

   // CPP -> OBJ
   {
      auto cxxflags = std::make_shared<fs_argument_writer>("cxxflags", e.feature_registry());
      cxxflags->add("<rtti>off", "-fno-rtti").
                add("<cxxstd>98", "-std=c++98").
                add("<cxxstd>03", "-std=c++03").
                add("<cxxstd>11", "-std=c++11").
                add("<cxxstd>14", "-std=c++14").
                add("<cxxstd>17", "-std=c++17").
                add("<cxxstd>2a", "-std=c++2a");

      auto cpp_input = std::make_shared<source_argument_writer>("cpp_input", e.get_type_registry().get(types::CPP), /*exact_type=*/false, source_argument_writer::FULL_PATH);
      cmdline_builder obj_cmd(td.compiler_.string() +
                              " -x c++ -c -ftemplate-depth-128 $(common-compiler-flags) $(cxxflags) $(user_cxx_flags) $(generated-includes) $(includes) $(defines) -o \"$(obj_product)\" $(cpp_input)");
      obj_cmd += common_compiler_flags;
      obj_cmd += cxxflags;
      obj_cmd += user_cxx_flags;
      obj_cmd += cpp_input;
      obj_cmd += generated_includes;
      obj_cmd += includes;
      obj_cmd += defines;
      obj_cmd += obj_product;
      auto obj_action = std::make_shared<cmdline_action>("compile-c++", obj_product);
      *obj_action += obj_cmd;
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ)));

      add_compile_generators(e, obj_action, [&](const build_action_ptr& compile_action) {
         return boost::make_unique<gcc_generator>(e, generator_prefix + ".compiler.cpp", source, target, false, compile_action, generator_condition, td.cxx_flags_);
      });
   }

   // ... -> SHARED_LIB
   {
      auto obj_sources = std::make_shared<source_argument_writer>("obj_sources", e.get_type_registry().get(types::OBJ));
      auto shared_lib_product = std::make_shared<product_argument_writer>("shared_lib_product", e.get_type_registry().get(types::SHARED_LIB));
      auto libraries_writer = std::make_shared<unix_libraries_argument_writer>("libraries", linker_type::GNU, e);
      cmdline_builder shared_lib_cmd(td.linker_.string() + " -shared $(link_flags) $(user_linkflags) $(searched_lib_searched_dirs) -o \"$(shared_lib_product)\" $(obj_sources) $(libraries)\n");
      shared_lib_cmd += link_flags;
      shared_lib_cmd += user_linkflags;
      shared_lib_cmd += searched_lib_searched_dirs;
      shared_lib_cmd += obj_sources;
      shared_lib_cmd += libraries_writer;
      shared_lib_cmd += shared_lib_product;

      auto shared_lib_action = std::make_shared<cmdline_action>("link-shared-lib", shared_lib_product);
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

      auto g = boost::make_unique<exe_and_shared_lib_generator>(e, generator_prefix + ".linker.shared_lib", source, target, true, shared_lib_action, generator_condition, td.link_flags_);
      e.generators().insert(std::move(g));
   }

   // ... -> EXE
   {
      auto ld_library_path_dirs = std::make_shared<free_feature_arg_writer>("ld_library_path_dirs", e.feature_registry(), "search", std::string(), std::string(), ":");
      auto obj_sources = std::make_shared<source_argument_writer>("obj_sources", e.get_type_registry().get(types::OBJ));
      auto exe_product = std::make_shared<product_argument_writer>("exe_product", e.get_type_registry().get(types::EXE));
      auto libraries_writer = std::make_shared<unix_libraries_argument_writer>("libraries", linker_type::GNU, e);
      auto exe_action = std::make_shared<cmdline_action>("link-exe", exe_product);
      cmdline_builder exe_cmd("LD_LIBRARY_PATH=$(ld_library_path_dirs):LD_LIBRARY_PATH " + td.linker_.string() + " $(link_flags) $(user_linkflags) $(searched_lib_searched_dirs) -o \"$(exe_product)\" $(obj_sources) $(libraries)\n");

      exe_cmd += link_flags;
      exe_cmd += user_linkflags;
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

      add_link_generators(e, exe_action, [&](const build_action_ptr& link_action) {
         return boost::make_unique<exe_and_shared_lib_generator>(e, generator_prefix + ".linker.exe", source, target, true, link_action, generator_condition, td.link_flags_);
      });
   }

   // ... -> STATIC_LIB
   {
      auto obj_sources = std::make_shared<source_argument_writer>("obj_sources", e.get_type_registry().get(types::OBJ));
      auto static_lib_product = std::make_shared<product_argument_writer>("static_lib_product", e.get_type_registry().get(types::STATIC_LIB));
      cmdline_builder static_lib_cmd(td.librarian_.string() + " $(user_archive_flags) rc $(static_lib_product) $(obj_sources)");

      static_lib_cmd += static_lib_product;
      static_lib_cmd += obj_sources;
      static_lib_cmd += user_archive_flags;

      auto static_lib_action = std::make_shared<cmdline_action>("link-static-lib", static_lib_product);
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
      auto g = boost::make_unique<static_lib_generator>(e, generator_prefix + ".linker.static_lib", source, target, true, static_lib_action, generator_condition);
      e.generators().insert(std::move(g));
   }

   register_configured(td.version_, *td.constraints_, make_toolset_info(td));
}

void gcc_toolset::use_toolset_rule(invocation_context& ctx,
                                   const parscore::identifier* version,
                                   const location_t* path_to_cxx_compiler,
                                   const location_t* path_to_linker,
                                   const location_t* path_to_ar,
                                   const parscore::identifier* c_flags,
                                   const parscore::identifier* cxx_flags,
                                   const parscore::identifier* link_flags,
                                   const feature_set* constraints) {
   if (!version && (path_to_cxx_compiler || path_to_linker || path_to_ar))
      throw ast2objects_semantic_error(ctx.rule_location_, "Toolset version expected when you specify path to compiler/linker/...");

   if (path_to_cxx_compiler && !path_to_cxx_compiler->has_root_path())
      throw ast2objects_semantic_error(ctx.rule_location_, "Compiler path should be absolute");

   if (path_to_linker && !path_to_linker->has_root_path())
      throw ast2objects_semantic_error(ctx.rule_location_, "Linker path should be absolute");

   if (path_to_ar && !path_to_ar->has_root_path())
      throw ast2objects_semantic_error(ctx.rule_location_, "Librarian path should be absolute");

   toolset_data td;

   if (!version)
      td.version_ = "system";
   else {
      td.version_ = version->to_string();
      td.version_suffix_ = "-" + td.version_;
   }

   if (path_to_cxx_compiler)
      td.compiler_ = *path_to_cxx_compiler;
   else
      td.compiler_ = "/usr/bin/g++" + td.version_suffix_;

   if (path_to_linker)
      td.linker_ = *path_to_linker;
   else
      td.linker_ = "/usr/bin/g++" + td.version_suffix_;

   if (path_to_ar)
      td.librarian_ = *path_to_ar;
   else
      td.librarian_ = "/usr/bin/gcc-ar" + td.version_suffix_;

   feature_registry& fr = ctx.current_project_.get_engine().feature_registry();

   if (c_flags) {
      td.c_flags_ = fr.make_set();
      td.c_flags_->join("cflags", c_flags->to_string().c_str());
   } else
      td.c_flags_ = nullptr;

   if (cxx_flags) {
      td.cxx_flags_ = fr.make_set();
      td.cxx_flags_->join("cxxflags", cxx_flags->to_string().c_str());
   } else
      td.cxx_flags_ = nullptr;

   if (link_flags) {
      td.link_flags_ = fr.make_set();
      td.link_flags_->join("linkflags", link_flags->to_string().c_str());
   } else
      td.link_flags_ = nullptr;

   if (constraints)
      td.constraints_ = constraints;
   else
      td.constraints_ = fr.make_set();

   if (is_already_configured(td.version_, *td.constraints_))
      throw ast2objects_semantic_error(ctx.rule_location_, "Same version with similar constraints already configured");

   init_toolset(ctx.current_project_.get_engine(), td);
}

void gcc_toolset::configure(engine& e,
                            const std::string& version) {
   auto td = toolset_data{version, "-" + version, "/usr/bin/g++-" + version, "/usr/bin/g++-" + version, "/usr/bin/gcc-ar-" + version};
   td.c_flags_ = nullptr;
   td.cxx_flags_ = nullptr;
   td.link_flags_ = nullptr;
   td.constraints_ = e.feature_registry().make_set();
   init_toolset(e, td);
}

static auto gcc_version_pattern = std::regex("gcc-(.*)");
void gcc_toolset::autoconfigure(engine& e) {
   auto default_gcc = fs::path("/usr/bin/gcc");
   if (fs::exists(default_gcc) && fs::is_symlink(default_gcc)) {
      // lets where it points to figure out exact version
      auto gcc = fs::read_symlink(default_gcc).filename();

      std::smatch m;
      if (!std::regex_match(gcc.string(), m, gcc_version_pattern))
          return;

      std::string version = m[1];
      configure(e, version);
   }
}

}
