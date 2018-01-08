#if !defined(h_4b03135f_1e7a_4294_b464_d7802b6e8628)
#define h_4b03135f_1e7a_4294_b464_d7802b6e8628

#include <hammer/core/basic_build_target.h>

namespace hammer
{
   class fake_build_target : public basic_build_target
   {
      public:
         fake_build_target(const main_target* mt,
                     const build_node::sources_t& sources,
                     const std::string& name,
                     const target_type* t, 
                     const feature_set* f);
         const location_t& location() const override;
			void clean(const build_environment& environment) const override {}

      protected:
         void timestamp_info_impl() const override;
      
      private:
         build_node::sources_t sources_;
   };
}

#endif //h_4b03135f_1e7a_4294_b464_d7802b6e8628
