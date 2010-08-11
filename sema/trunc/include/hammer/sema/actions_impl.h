#if !defined(h_7858a0f2_780c_4919_9a5a_86104e12c129)
#define h_7858a0f2_780c_4919_9a5a_86104e12c129

#include <hammer/sema/actions.h>
#include <hammer/ast/context.h>

namespace hammer{namespace sema{

class actions_impl : public actions
{
   public:
      actions_impl(ast::context& ctx);
      virtual void on_begin_parse(ast::parser_context* ctx) const;
      
      virtual const ast::hamfile* 
         on_hamfile(const ast::project_def* p,
                    const ast::statements_t& statements) const;
      
      virtual const ast::project_def* 
         on_implicit_project_def() const;

      virtual const ast::project_def* 
         on_explicit_project_def(const parscore::identifier& name, 
                                 const ast::expressions_t& expressions) const;

      virtual const ast::expression* 
         on_empty_expr() const;

      virtual const ast::expression* 
         on_list_of(const ast::expressions_t& e) const;

      virtual const ast::path_like_seq* 
         on_path_like_seq(const parscore::identifier& first, 
                          const parscore::identifier& last) const;

      virtual const ast::expression* 
         on_target_or_rule_call(const parscore::identifier& rule_name, 
                                const ast::expressions_t& arguments) const;

      virtual const ast::feature*
         on_feature(parscore::identifier name,
                    const ast::expression* value) const;

      virtual const ast::requirement_set* 
         on_requirement_set(const ast::requirements_t& requirements) const;

      virtual const ast::requirement* 
         on_simple_requirement(parscore::source_location public_tag_loc,
                               const ast::feature* value) const;

      virtual const ast::requirement* 
         on_conditional_requirement(parscore::source_location public_tag_loc,
                                    const ast::features_t& features,
                                    const ast::feature* value) const;
   
      virtual const ast::expression*
         on_target_ref(parscore::source_location public_tag,
                       const ast::path_like_seq* head,
                       const parscore::identifier& target_name,
                       const ast::requirement_set* requirements) const;

   private:
      ast::context& ctx_;
};

}}

#endif //h_7858a0f2_780c_4919_9a5a_86104e12c129
