#if !defined(h_aa33b781_7a6b_4989_b465_5494a19dc8f3)
#define h_aa33b781_7a6b_4989_b465_5494a19dc8f3

#include <hammer/core/typed_meta_target.h>

namespace hammer{
   
class testing_intermediate_meta_target : public typed_meta_target
{
   public:
      testing_intermediate_meta_target(hammer::project* p, 
                                       const pstring& name, 
                                       const requirements_decl& req, 
                                       const requirements_decl& usage_req,
                                       const target_type& t);
   protected:
      virtual sources_decl compute_additional_sources(const main_target& owner) const;
      virtual bool is_cachable(const main_target* owner) const { return false; }
};

}

#endif //h_aa33b781_7a6b_4989_b465_5494a19dc8f3
