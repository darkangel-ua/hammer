#pragma once

#include <list>

namespace hammer
{
   class pool
   {
      public:
         pool();
         void* malloc(size_t size);
         ~pool();
      
      private:
         std::list<char*> m_;
   };
}