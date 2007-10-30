#include "stdafx.h"
#include "feature_registry.h"

using namespace std;

namespace hammer{

   feature_registry::feature_registry()
   {

   }

   feature_registry::~feature_registry()
   {

   }

   void feature_registry::add_def(std::auto_ptr<feature_def> def)
   {
      pair<defs_t::iterator, bool> p = defs_.insert(def->name(), def.get());
      if (!p.second)
         throw std::runtime_error("feature_def with name '" + def->name() + "' already registered");
   }
}
