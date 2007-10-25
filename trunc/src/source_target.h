#pragma once
#include "pstring.h"
#include "basic_target.h"

namespace hammer
{
   class meta_target;
   class type;
   class engine;

   class source_target : public basic_target
   {
      public:  
         source_target(const meta_target* mt, const pstring& name, const hammer::type* t);
         const pstring& name() const { return name_; }
         void* operator new(size_t size, engine* e);
         void operator delete(void* mem, engine*) {};

      private:
         const meta_target* meta_target_;
         pstring name_;
   };
}
