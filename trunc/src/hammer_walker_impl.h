#if !defined(h_260de696_83c3_4919_8fd2_1f928a4da80d)
#define h_260de696_83c3_4919_8fd2_1f928a4da80d

#include <antlr3basetree.h>

#ifdef __cplusplus
extern "C"
{
#endif
   void* hammer_rule_call(void* context, const char* rule_name, void* args_list);
   void  hammer_delete_rule_result(void* result);
   void* hammer_make_args_list(void* context);
   void* hammer_make_null_arg();
   void* hammer_make_string_list();
   void* hammer_make_feature_list(void* context);
   void* hammer_make_path();
   void hammer_add_arg_to_args_list(void* args_list, void* arg);
   void hammer_add_id_to_string_list(void* context, void* string_list, const char* id);
   void hammer_add_feature_to_list(void* context, void* args_list, const char* feature_name, const char* feature_value);
   void hammer_add_feature_argument(void* context, void* args_list, const char* feature_name, const char* feature_value);
   void hammer_add_string_arg_to_args_list(void* context, void* args_list, const char* id);
   void hammer_add_to_path(void* p, const char* token);

   // requirements
   void* hammer_make_requirements_decl();
   void* hammer_make_project_requirements_decl(const char* id, void* rdecl);
   void* hammer_make_requirements_decl_arg(void* rdecl);
   void* hammer_make_project_requirements_decl_arg(void* pdecl);
   void* hammer_make_requirements_condition();
   void* hammer_make_path_arg(void* p);
   void* hammer_make_feature_set_arg(void* fs);
   void* hammer_make_feature_arg(void* f);

   void* hammer_create_feature(void* context, const char* feature_name, const char* feature_value);
   void hammer_add_conditional_to_rdecl(void* condition, void* rdecl);
   void hammer_add_feature_to_rdecl(void* feature, void* rdecl);
   void hammer_set_condition_result(void* condition, void* feature);
   void hammer_add_feature_to_condition(void* feature, void* condition);
   void* hammer_make_sources_decl();
   void hammer_add_source_to_sources_decl(void* result, void* sd);
   void* hammer_make_sources_decl_arg(void* s);
   void hammer_add_rule_result_to_source_decl(void* rule_result, void* sources);
   void* hammer_make_source_decl();
   void hammer_source_decl_set_target_path(void* context, void* sd, void* tp);
   void hammer_source_decl_set_target_name(void* context, void* sd, const char* id);
   void hammer_source_decl_set_target_properties(void* sd, void* fs);
   void* hammer_make_target_path();
   void hammer_add_to_target_path(void* context, void* tp, pANTLR3_BASE_TREE node);

   void* hammer_make_feature_set(void* context);
   void hammer_add_feature_to_feature_set(void* feature_set, void* feature);
   void hammer_feature_set_dependency_data(void* f, void* sd);
#ifdef __cplusplus
};
#endif

#endif //h_260de696_83c3_4919_8fd2_1f928a4da80d
