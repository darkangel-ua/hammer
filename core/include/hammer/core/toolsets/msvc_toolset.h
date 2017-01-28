#if !defined(h_4600a2af_9f05_4199_bb10_2b42d6ca6b22)
#define h_4600a2af_9f05_4199_bb10_2b42d6ca6b22

#include <hammer/core/toolset.h>

namespace hammer
{
   class msvc_toolset : public toolset
   {
      public:
         msvc_toolset();
         ~msvc_toolset();
         void autoconfigure(engine& e) const override;

      protected:
         void init_impl(engine& e,
                        const std::string& version_id = std::string(),
                        const location_t* toolset_home = NULL) const override;

      private:
         struct impl_t;
         impl_t* impl_;

         void init(engine& e,
                   const std::string& version_id,
                   const location_t& toolset_home) const;
   };
}

#endif //h_4600a2af_9f05_4199_bb10_2b42d6ca6b22
