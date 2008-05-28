#ifndef __HAMMER_WALKER_IMPL__
#define __HAMMER_WALKER_IMPL__

#ifdef __cplusplus
extern "C"
{
#endif

/*
   void hammer_make_project(void* context, const char* project_id);
   void* hammer_add_lib_meta_target(void* context, const char* target_id);
   void* hammer_add_exe_meta_target(void* context, const char* target_id);
   void hammer_add_target_to_mt(void* context, void* mt, const char*);
*/

   void hammer_rule_call(void* context, const char* rule_name, void* args_list);
   void* hammer_make_args_list(void* context);
   void* hammer_make_null_arg();
   void* hammer_make_string_list();
   void* hammer_make_feature_list(void* context);
   void hammer_add_arg_to_args_list(void* args_list, void* arg);
   void hammer_add_id_to_string_list(void* context, void* string_list, const char* id);
   void hammer_add_feature_to_list(void* context, void* args_list, const char* feature_name, const char* feature_value);
   
#ifdef __cplusplus
};
#endif

#endif