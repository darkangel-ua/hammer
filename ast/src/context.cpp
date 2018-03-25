#include <hammer/ast/context.h>

namespace hammer{ namespace ast{

context::context()
{

}

void* context::allocate(std::size_t bytes)
{
   char* m = new char[bytes];
   allocated_blocks_.push_back(m);
   return m;
}

context::~context()
{
   for (char* i : allocated_blocks_)
      delete[] i;
}

}}
