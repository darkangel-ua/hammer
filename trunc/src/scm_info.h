#if !defined(h_90141c4b_aabf_463c_be8c_02153fdbd614)
#define h_90141c4b_aabf_463c_be8c_02153fdbd614

#include "pstring.h"

namespace hammer
{
   class scm_info
   {
      public:
         scm_info() {}
         scm_info(const pstring& scm_url, 
                  const pstring& scm_client_name) 
                 :
                  scm_url_(scm_url),
                  scm_client_name_(scm_client_name)
         {}
         
         pstring scm_url_;
         pstring scm_client_name_;
   };
}

#endif //h_90141c4b_aabf_463c_be8c_02153fdbd614
