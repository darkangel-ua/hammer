#if !defined(h_dbbb68bd_8832_4d6c_aa77_baeb28bdbc79)
#define h_dbbb68bd_8832_4d6c_aa77_baeb28bdbc79

#include <hammer/core/toolset.h>

namespace hammer
{
   class gcc_toolset : public toolset
   {
      public:
         struct gcc_install_data
         {
            std::string version_;
            location_t compiler_;
            location_t linker_;
            location_t librarian_;
         };

         gcc_toolset();
         virtual void autoconfigure(engine& e) const;
      
      protected:
         virtual void init_impl(engine& e, const std::string& version_id = std::string(),
                                const location_t* toolset_home = NULL) const;
         virtual gcc_install_data resolve_install_data(const location_t* toolset_home) const;
   };
}

#endif //h_dbbb68bd_8832_4d6c_aa77_baeb28bdbc79
