#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
   void check_target(const char* target_name);
   void* get_target(const char* id, void* t, int is_top);
#ifdef __cplusplus
};
#endif
