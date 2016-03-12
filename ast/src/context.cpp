#include "stdafx.h"
#include <hammer/ast/context.h>

namespace hammer{ namespace ast{

void* context::allocate(size_t bytes, size_t alignment)
{
   char* m = new char[bytes];
   allocated_blocks_.push_back(m);
   return m;
}

context::~context()
{
   for(std::vector<char*>::iterator i = allocated_blocks_.begin(), last = allocated_blocks_.end(); i != last; ++i)
      delete[] *i;
}

}}