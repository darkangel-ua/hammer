#ifndef __HAMMER_WALKER_IMPL__
#define __HAMMER_WALKER_IMPL__

#ifdef __cplusplus
extern "C"
{
#endif

   void hammer_make_project(void* context, const char* project_id);
   void* hammer_add_lib_meta_target(void* context, const char* target_id);
   void* hammer_add_exe_meta_target(void* context, const char* target_id);
   void hammer_add_target_to_mt(void* context, void* mt, const char*);

#ifdef __cplusplus
};
#endif

#endif