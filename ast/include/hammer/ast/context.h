#pragma once
#include <vector>

namespace hammer { namespace ast {

class context {
   public:
		context();
		context(const context&) = delete;
      context& operator = (const context&) = delete;

      ~context();
      void* allocate(std::size_t bytes);

	private:
      std::vector<char*> allocated_blocks_;
};

}}

inline
void *operator new(std::size_t bytes,
                   hammer::ast::context& c)
{
   return c.allocate(bytes);
}

inline
void operator delete(void* p,
                     hammer::ast::context& c)
{}
