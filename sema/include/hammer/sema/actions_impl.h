#pragma once
#include <unordered_set>
#include <hammer/sema/actions.h>
#include <hammer/ast/context.h>
#include <hammer/core/rule_manager.h>

namespace hammer {

class diagnostic;

}

namespace hammer { namespace ast {

class public_expr;

}}

namespace hammer { namespace sema {

class actions_impl : public actions {
   public:
      struct environment {
            virtual bool known_feature(const parscore::identifier& name) const = 0;
            virtual ~environment() {}
      };

      actions_impl(ast::context& ctx,
                   environment& env,
                   const hammer::rule_manager& rule_manager,
                   hammer::diagnostic& diag);
      
   private:
      const ast::hamfile*
      on_hamfile(const ast::statements_t& statements) override;
      
      const ast::expression*
      on_empty_expr(const parscore::identifier& next_token) override;

      const ast::expression*
      on_named_expr(const parscore::identifier& name,
                    const ast::expression* value) override;

      const ast::expression*
      on_public_expr(const parscore::identifier& tag,
                     const ast::expression* value) override;

      const ast::expression*
      on_list_of(const ast::expressions_t& e) override;

      const ast::path*
      on_path(const parscore::token& root_name,
              const ast::expressions_t& elements,
              const parscore::token& trailing_slash) override;

      const ast::expression*
      on_id(const parscore::identifier& id) override;

      const ast::expression*
      on_rule_invocation(const parscore::identifier& rule_name,
                         const ast::expressions_t& arguments) override;

      const ast::statement*
      on_top_level_rule_invocation(const parscore::source_location explicit_tag,
                                   const parscore::source_location local_tag,
                                   const parscore::identifier& rule_name,
                                   const ast::expressions_t& arguments) override;

      const ast::feature*
      on_feature(parscore::identifier name,
                 const ast::expression* value) override;

      const ast::logical_or*
      on_logical_or(const ast::expression* left,
                    const ast::expression* right) override;

      const ast::logical_and*
      on_logical_and(const ast::expression* left,
                     const ast::expression* right) override;

      const ast::condition_expr*
      on_condition(const ast::expression* condition,
                   const ast::expression* result) override;

      const ast::expression*
      on_target_ref(parscore::source_location public_tag,
                    parscore::source_location project_local_ref_tag,
                    const ast::path* target_path,
                    const parscore::identifier& target_name,
                    const ast::features_t& build_request) override;

      const ast::expression*
      on_struct(parscore::source_location start_brace,
                const ast::expressions_t& fields) override;

   private:
      ast::expressions_t
      process_arguments(const parscore::identifier& rule_name,
                        const rule_declaration& rule_decl,
                        const ast::expressions_t& arguments);
      ast::expressions_t
      process_arguments_impl(const char* prefix,
                             const char* kind,
                             const char* argument_or_field,
                             const char* rule_or_struct_name,
                             parscore::source_location rule_or_struct_name_loc,
                             rule_declaration::const_iterator args_decl_first,
                             rule_declaration::const_iterator args_decl_last,
                             ast::expressions_t::const_iterator arguments_first,
                             ast::expressions_t::const_iterator arguments_last);

      const ast::expression*
      process_one_arg(const char* prefix,
                      const rule_argument& ra,
                      const ast::expression* arg);

      const ast::expression*
      process_identifier_arg(const char* prefix,
                             const rule_argument& ra,
                             const ast::expression* arg);
      const ast::expression*
      process_feature_arg(const char* prefix,
                          const rule_argument& ra,
                          const ast::expression* arg);
      const ast::expression*
      process_feature_set_arg(const char* prefix,
                              const rule_argument& ra,
                              const ast::expression* arg);
      const ast::expression*
      process_sources_arg(const rule_argument& ra,
                          const ast::expression* arg);
      const ast::expression*
      process_requirements_decl_arg(const char* prefix,
                                    const rule_argument& ra,
                                    const ast::expression* arg);
      const ast::expression*
      process_usage_requirements_arg(const rule_argument& ra,
                                     const ast::expression* arg);
      const ast::expression*
      process_path_arg(const char* prefix,
                       const rule_argument& ra,
                       const ast::expression* arg);
      const ast::expression*
      process_wcpath_arg(const char* prefix,
                         const rule_argument& ra,
                         const ast::expression* arg);
      const ast::expression*
      process_feature_of_feature_set_arg(const char* prefix,
                                         const rule_argument& ra,
                                         const ast::expression* arg);
      const ast::expression*
      process_target_ref_arg(const char* prefix,
                             const rule_argument& ra,
                             const ast::expression* arg);
      const ast::expression*
      process_target_ref_mask_arg(const char* prefix,
                                  const rule_argument& ra,
                                  const ast::expression* arg);
      const ast::expression*
      process_condition(const rule_argument& ra,
                        const ast::expression* e);
      const ast::expression*
      process_struct_arg(const char* prefix,
                         const rule_argument& ra,
                         const rule_argument_struct_desc& rasd,
                         const ast::expression* arg);
      const ast::expression*
      process_one_or_list_arg(const char* prefix,
                              const rule_argument& ra,
                              const rule_argument_type_desc& nested_type,
                              const ast::expression* arg);
      const ast::expression*
      wrap_public(const ast::expression* e,
                  const ast::public_expr* pe);

      bool known_feature(const parscore::identifier& id) const;

   private:
      const environment& env_;
      const hammer::rule_manager& rule_manager_;
      hammer::diagnostic& diag_;
      bool first_rule_in_file_ = true;
      std::unordered_set<std::string> features_;
};

}}
