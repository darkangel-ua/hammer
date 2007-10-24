#pragma once

#include <list>

namespace hammer
{
   class pool
   {
      public:
         pool();
         void* malloc(unsigned int size);
         ~pool();
      
      private:
         std::list<char*> m_;
   };
}