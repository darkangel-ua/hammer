#if !defined(h_0483f3e2_249b_44b1_b048_e8bbbf8ba158)
#define h_0483f3e2_249b_44b1_b048_e8bbbf8ba158

#include <vector>
#include <memory>
#include <cassert>

namespace hammer{
   class rule_manager;
   class diagnostic;
}

namespace hammer{ namespace ast{

class hamfile;

class parser_context
{
   public:
      virtual ~parser_context() {}
};

class context
{
   public:
      context(const hammer::rule_manager& rule_manager,
              hammer::diagnostic& diag)
         : rule_manager_(rule_manager),
           diag_(diag)
      {
      }

      context(const context&) = delete;

      ~context();
      void* allocate(std::size_t bytes,
                     std::size_t alignment);

      const hammer::rule_manager& rule_manager_;
      hammer::diagnostic& diag_;
      const hammer::ast::hamfile* hamfile_;

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
