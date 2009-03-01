#if !defined(h_4600a2af_9f05_4199_bb10_2b42d6ca6b22)
#define h_4600a2af_9f05_4199_bb10_2b42d6ca6b22

#include <hammer/core/toolset.h>

namespace hammer
{
   class msvc_toolset : public toolset
   {
      public:
         struct msvc_8_0_data
         {
            location_t setup_script_;
            location_t compiler_;
            location_t linker_;
            location_t librarian_;
            location_t manifest_tool_;
         };

         msvc_toolset();
      
      protected:
         virtual void init_impl(engine& e, const std::string& version_id = std::string(),
                                const location_t* toolset_home = NULL) const;
         virtual msvc_8_0_data resolve_8_0_data(const location_t* toolset_home) const;

      private:
         void init_8_0(engine& e, const location_t* toolset_home) const;
   };
}

#endif //h_4600a2af_9f05_4199_bb10_2b42d6ca6b22
