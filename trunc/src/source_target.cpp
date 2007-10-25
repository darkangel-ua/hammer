#include "stdafx.h"
#include "source_target.h"
#include "engine.h"

namespace hammer{

   source_target::source_target(const meta_target* mt, const pstring& name, const hammer::type* t)
      : basic_target(t), meta_target_(mt), name_(name)
   {
   }

   void* source_target::operator new(size_t size, engine* e)
   {
      void* m = e->pstring_pool().malloc(size);
      return m;
   }
}
