#include "stdafx.h"
#include "msvc_generator.h"
#include <boost/assign/list_of.hpp>
#include "generator.h"
#include "types.h"
#include "engine.h"
#include "type_registry.h"
#include "generator_registry.h"

using namespace boost::assign;
using namespace std;

namespace hammer{

void add_msvc_generators(engine& e, generator_registry& gr)
{
   {
      generator::consumable_types source;
      generator::prodused_types target;
      source.push_back(generator::consumable_type(&e.get_type_registry().resolve_from_name(types::CPP), 1, 0));
      target.push_back(generator::produced_type(&e.get_type_registry().resolve_from_name(types::OBJ), 1));
      generator g("msvc.cpp.compiler", source, target);
      e.generators().insert(g);
   }
  
   { 
      generator::consumable_types source;
      generator::prodused_types target;
      source.push_back(generator::consumable_type(&e.get_type_registry().resolve_from_name(types::OBJ), 0, 0));
      target.push_back(generator::produced_type(&e.get_type_registry().resolve_from_name(types::EXE), 1));
      generator g("msvc.cpp.linker", source, target);
      e.generators().insert(g);
   }
}

}