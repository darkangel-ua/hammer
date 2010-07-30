#if !defined(h_0483f3e2_249b_44b1_b048_e8bbbf8ba158)
#define h_0483f3e2_249b_44b1_b048_e8bbbf8ba158

#include <vector>
#include <cassert>

namespace hammer{ namespace core{ namespace rules{
   class manager;
}}}

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
      context(const core::rules::manager& rule_manager)
         : rule_manager_(rule_manager)
      {
      }
      ~context();
      const hammer::ast::hamfile* hamfile() const { return hamfile_; }
      void* allocate(size_t bytes, size_t alignment);
      void set_parser_context(std::auto_ptr<parser_context>& p_ctx)
      {
         parser_context_ = p_ctx;
      }

   private:
      const core::rules::manager& rule_manager_;
      std::vector<char*> allocated_blocks_;
      const hammer::ast::hamfile* hamfile_;
      std::auto_ptr<parser_context> parser_context_;
};

}}

inline void *operator new(size_t bytes, 
                          hammer::ast::context& c) throw () 
{
   return c.allocate(bytes, 0);
}

inline void operator delete(void*, 
                             hammer::ast::context&) throw () 
{
   assert(false && "You should never call this operator!");
}

#endif //h_0483f3e2_249b_44b1_b048_e8bbbf8ba158
