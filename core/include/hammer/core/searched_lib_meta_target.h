#if !defined(h_3aa9daa7_6256_4cdb_a4c2_cced3b3c758b)
#define h_3aa9daa7_6256_4cdb_a4c2_cced3b3c758b

#include <hammer/core/typed_meta_target.h>

namespace hammer{

   class searched_lib_meta_target : public typed_meta_target
   {
      public:
         searched_lib_meta_target(hammer::project* p, 
                                  const std::string& name,
                                  const std::string& lib_name,
                                  const requirements_decl& props,
                                  const requirements_decl& usage_req,
                                  const target_type& t);
      protected:
         main_target* construct_main_target(const main_target* owner,
			                                   const feature_set* properties) const override;
      
      private:
         const std::string lib_name_;
   };
}

#endif //h_3aa9daa7_6256_4cdb_a4c2_cced3b3c758b
