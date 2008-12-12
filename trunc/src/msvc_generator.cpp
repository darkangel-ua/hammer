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

using namespace boost::assign;
using namespace std;

namespace hammer{

void add_msvc_generators(engine& e, generator_registry& gr)
{
   e.feature_registry().get_def("toolset").extend_legal_values("msvc");

   // CPP -> OBJ
   {
      auto_ptr<fs_argument_writer> compile_flags(new fs_argument_writer("compile_flags", e.feature_registry()));
      compile_flags->add("<optimization>speed", "/O2").
                     add("<optimization>space", "/O1");

      auto_ptr<source_argument_writer> cpp_input(new source_argument_writer("cpp_input", e.get_type_registry().get(types::CPP)));
      auto_ptr<product_argument_writer> obj_output(new product_argument_writer("obj_output", e.get_type_registry().get(types::OBJ)));
      cmdline_builder setup_vars("\"C:\\Program Files\\Microsoft Visual Studio 8\\VC\\bin\\vcvars32.bat\"");
      cmdline_builder obj_cmd("cl.exe $(compile_flags) \"$(cpp_input)\" -Fo\"$(obj_output)\"");
      obj_cmd += compile_flags;
      obj_cmd += cpp_input;
      obj_cmd += obj_output;
      auto_ptr<cmdline_action> obj_action(new cmdline_action);
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
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.c.compiler", source, target, false));
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
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().get(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().get(types::EXE), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.exe.linker", source, target, true));
      e.generators().insert(g);
   }

   { 
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
      e.generators().insert(g);
   }

   { 
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
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.shared_lib.linker", source, target, true));
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