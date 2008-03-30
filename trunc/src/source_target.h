#pragma once
#include "pstring.h"
#include "basic_target.h"

namespace hammer
{
   class main_target;
   class type;
   class engine;
   class feature_set;

   class source_target : public basic_target
   {
      public:  
         source_target(const main_target* mt, const pstring& name, 
                       const hammer::type* t, const feature_set* f);
         void* operator new(size_t size, engine* e);
         void operator delete(void* mem, engine*) {};
         virtual std::vector<basic_target*> generate();
   };
}
