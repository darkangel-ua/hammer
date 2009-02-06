#include "stdafx.h"
#include <hammer/core/pool.h>

namespace hammer{

   pool::pool()
   {

   }
   
   void* pool::malloc(size_t size)
   {
      char* p = new char[size];
      try
      {
         m_.insert(m_.end(), p);
         return p;
      }catch(...)
      {
         delete[] p;
         throw;
      }
   }

   pool::~pool()
   {
      for(std::list<char*>::iterator i = m_.begin(), last = m_.end(); i != last; ++i)
         delete[] *i;
   }
}