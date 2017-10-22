#if !defined(h_0483f3e2_249b_44b1_b048_e8bbbf8ba158)
#define h_0483f3e2_249b_44b1_b048_e8bbbf8ba158

#include <vector>

namespace hammer{ namespace ast{


class context
{
   public:
		context();
		context(const context&) = delete;
      context& operator = (const context&) = delete;

      ~context();
      void* allocate(std::size_t bytes,
                     std::size_t alignment);

	private:
      std::vector<char*> allocated_blocks_;
};

}}

inline void *operator new(std::size_t bytes,
                          hammer::ast::context& c) throw () 
{
   return c.allocate(bytes, 0);
}

#endif
