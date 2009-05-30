#if !defined(h_82225d39_86ef_4560_a63e_08466d9599bc)
#define h_82225d39_86ef_4560_a63e_08466d9599bc

#include <antlr3.h>

#ifdef __cplusplus
extern "C"
{
#endif
   void check_target(const char* target_name);
   void* get_target(const char* id, void* t, int is_top);
   void check_type(void* e, void *t, const char* type_id);
   void* get_features(void* t);
   void check_feature(void* e, void* t, void* f, const pANTLR3_COMMON_TOKEN name, const pANTLR3_COMMON_TOKEN value);
   void check_not_feature(void* e, void* t, void* f, const char* name, const char* value);
   void check_location(void* t, const char* location);
#ifdef __cplusplus
};
#endif

#endif //h_82225d39_86ef_4560_a63e_08466d9599bc
