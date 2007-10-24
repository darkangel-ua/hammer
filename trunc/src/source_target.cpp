#include "stdafx.h"
#include "source_target.h"
#include "engine.h"

namespace hammer{

   source_target::source_target(const meta_target* mt, const pstring& name, const type* t)
      : meta_target_(mt), name_(name), type_(t)
   {
   }

   void* source_target::operator new(size_t size, engine* e)
   {
      void* m = new char[size];
      e->insert(static_cast<basic_target*>(m));
      return m;
   }
}
