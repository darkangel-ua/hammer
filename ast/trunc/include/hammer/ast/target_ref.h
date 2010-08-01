#if !defined(h_bd428576_c9b2_4eb5_8a5a_1fe3dd0e85cb)
#define h_bd428576_c9b2_4eb5_8a5a_1fe3dd0e85cb

#include <hammer/ast/expression.h>

namespace hammer{namespace ast{

class target_ref : public expression
{
   public:
      target_ref(const path_like_seq* head,
                 const parscore::identifier& target_name,
                 const feature_set* properties)
         : head_(head),
           target_name_(target_name),
           properties_(properties)
      {}

      const path_like_seq* head() const { return head_; }
      const parscore::identifier& target_name() const { return target_name_; }
      const feature_set* properties() const { return properties_; }
      bool has_target_name() const;
      virtual bool accept(visitor& v) const;
   
   private:
      const path_like_seq* head_;
      parscore::identifier target_name_;
      const feature_set* properties_;
};

}}

#endif //h_bd428576_c9b2_4eb5_8a5a_1fe3dd0e85cb
