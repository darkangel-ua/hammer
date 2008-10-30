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

using namespace boost::assign;
using namespace std;

namespace hammer{

void add_msvc_generators(engine& e, generator_registry& gr)
{
   // CPP -> OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.cpp.compiler", source, target, false));
      e.generators().insert(g);
   }

   // CPP + H -> PCH + OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::CPP), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::PCH), 1));

      feature_set* constraints = e.feature_registry().make_set();
      constraints->join("__pch", NULL);
      auto_ptr<generator> g(new pch_generator(e, "msvc.cpp-pch.compiler", source, target, true, constraints));
      
      e.generators().insert(g);
   }

   // C -> OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.c.compiler", source, target, false));
      e.generators().insert(g);
   }

   // C + H -> PCH + OBJ
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::C), 1, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::PCH), 1));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));

      feature_set* constraints = e.feature_registry().make_set();
      constraints->join("__pch", NULL);
      auto_ptr<generator> g(new pch_generator(e, "msvc.c-pch.compiler", source, target, true, constraints));
      
      e.generators().insert(g);
   }

   // this is generator for obj meta target. Consume all input and transfer only obj types to result products
   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "obj meta target generator", source, target, true));
      e.generators().insert(g);
   }

   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::EXE), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.exe.linker", source, target, true));
      e.generators().insert(g);
   }

   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 1));
      auto_ptr<generator> g(new static_lib_generator(e, "msvc.static_lib.linker", source, target, true));
      e.generators().insert(g);
   }

   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::SHARED_LIB), 1));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.shared_lib.linker", source, target, true));
      e.generators().insert(g);
   }

   { 
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::H), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::STATIC_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::SEARCHED_LIB), 0, 0));
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 0, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::HEADER_LIB), 1));
      auto_ptr<generator> g(new header_lib_generator(e, "header_lib.linker", source, target));
      e.generators().insert(g);
   }

}

}