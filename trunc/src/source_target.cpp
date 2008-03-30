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

   std::vector<basic_target*> source_target::generate()
   {
      return std::vector<basic_target*>(1, this);
   }

}
