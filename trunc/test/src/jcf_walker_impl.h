#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
   void check_target(const char* target_name);
   void* get_target(const char* id, void* t, int is_top);
   void check_type(void* e, void *t, const char* type_id);
   void* get_features(void* t);
   void check_feature(void* f, const char* name, const char* value);
#ifdef __cplusplus
};
#endif