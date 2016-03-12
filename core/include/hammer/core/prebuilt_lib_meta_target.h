#if !defined(h_bb5afffa_edeb_4cc5_b803_e2707f4d9995)
#define h_bb5afffa_edeb_4cc5_b803_e2707f4d9995

#include <hammer/core/searched_lib_meta_target.h>

namespace hammer
{
   class prebuilt_lib_meta_target : public searched_lib_meta_target
   {
      public:
         prebuilt_lib_meta_target(hammer::project* p, 
                                  const pstring& name, 
                                  const pstring& lib_file_path,
                                  const requirements_decl& props,
                                  const requirements_decl& usage_req);
   };
}

#endif //h_bb5afffa_edeb_4cc5_b803_e2707f4d9995
