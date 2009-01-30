#if !defined(h_36226b9c_f805_440f_95a4_692de7558875)
#define h_36226b9c_f805_440f_95a4_692de7558875

#include "meta_target.h"

namespace hammer
{
   class alias_meta_target : public basic_meta_target
   {
      public:
         alias_meta_target(hammer::project* p, const pstring& name, 
                           const sources_decl& sources,
                           const requirements_decl& req,
                           const requirements_decl& usage_req);
      protected:
         virtual void instantiate_impl(const main_target* owner, 
                                       const feature_set& build_request,
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirements) const;
         
         virtual bool is_cachable(const main_target* owner) const 
         { 
            return false; 
         }
      
      private:
         virtual void transfer_sources(sources_decl* simple_targets, 
                                       meta_targets_t* meta_targets,
                                       const feature_set& build_request,
                                       const feature_set* additional_build_properties) const;
   };
}

#endif //h_36226b9c_f805_440f_95a4_692de7558875
