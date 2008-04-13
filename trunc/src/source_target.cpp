#include "stdafx.h"
#include "source_target.h"
#include "engine.h"

namespace hammer{

   source_target::source_target(const main_target* mt, const pstring& name, 
                                const hammer::type* t, const feature_set* f)
                               : basic_target(mt, name, t, f)
   {
   }

   void* source_target::operator new(size_t size, engine* e)
   {
      void* m = e->pstring_pool().malloc(size);
      return m;
   }

   std::auto_ptr<build_node> source_target::generate()
   {
      std::auto_ptr<build_node> result(new build_node);
      result->products_.push_back(this);
      return result;
   }
}
