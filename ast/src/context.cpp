#include <hammer/ast/context.h>

namespace hammer{ namespace ast{

context::context()
{

}

void* context::allocate(std::size_t bytes,
                        std::size_t alignment)
{
   static std::size_t c = 0;
   if (++c == 90) {
      c = c;
   }
   char* m = new char[bytes];
   allocated_blocks_.push_back(m);
   return m;
}

context::~context()
{
   std::size_t c = 0;
   for(std::vector<char*>::iterator i = allocated_blocks_.begin(), last = allocated_blocks_.end(); i != last; ++i, ++c)
      delete[] *i;
}

}}
