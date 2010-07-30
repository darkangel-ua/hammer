#if !defined(h_f169f8f8_dd1c_4f6a_9071_3a123619437f)
#define h_f169f8f8_dd1c_4f6a_9071_3a123619437f

#include <vector>
#include <hammer/ast/node.h>

namespace hammer{namespace ast{

class project_def;
class statement;

typedef std::vector<const statement*> statements_t;

class hamfile : public node
{
   public:
      hamfile(const project_def* p, 
              const statements_t& s)
         : project_(p), 
           statements_(s)
      {};

      virtual bool accept(visitor& v) const;
      
      const project_def* get_project_def() const { return project_; }

   private:
      const project_def* project_;
      statements_t statements_;
};

}}
#endif //h_f169f8f8_dd1c_4f6a_9071_3a123619437f
