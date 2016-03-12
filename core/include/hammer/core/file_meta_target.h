#if !defined(h_3bda6362_7fc5_48f7_9791_72655cf48f94)
#define h_3bda6362_7fc5_48f7_9791_72655cf48f94

#include <hammer/core/meta_target.h>

namespace hammer
{
   class file_meta_target : public meta_target
   {
      public:
         file_meta_target(hammer::project* p, 
                          const pstring& name, 
                          const pstring& filename,
                          const requirements_decl& req, 
                          const requirements_decl& usage_req);
      protected:
         virtual main_target* construct_main_target(const main_target* owner, const feature_set* properties) const;

      private:
         pstring filename_;
   };
}

#endif //h_3bda6362_7fc5_48f7_9791_72655cf48f94
