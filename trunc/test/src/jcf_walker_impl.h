#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
   void check_target(const char* target_name);
   void get_target(void* ctx_, const char* id, int is_top);
   void check_type(void* e, void *t, const char* type_id);
//   void get_features(void* ctx);
   void check_feature(void* ctx_, const char* name, const char* value);
   void check_location(void* ctx_, const char* location);
#ifdef __cplusplus
};
#endif
