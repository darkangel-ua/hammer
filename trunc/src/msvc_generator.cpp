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

using namespace boost::assign;
using namespace std;

namespace hammer{

void add_msvc_generators(engine& e, generator_registry& gr)
{
   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::CPP), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.cpp.compiler", source, target, false));
      e.generators().insert(g);
   }

   {
      generator::consumable_types_t source;
      generator::producable_types_t target;
      source.push_back(generator::consumable_type(e.get_type_registry().resolve_from_name(types::C), 1, 0));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::OBJ), 1));
      auto_ptr<generator> g(new generator(e, "msvc.c.compiler", source, target, false));
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
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::SHARED_LIB), 1));
      target.push_back(generator::produced_type(e.get_type_registry().resolve_from_name(types::IMPORT_LIB), 1));
      auto_ptr<generator> g(new exe_and_shared_lib_generator(e, "msvc.shared_lib.linker", source, target, true));
      e.generators().insert(g);
   }

}

}