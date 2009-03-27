#if !defined(h_341a3733_d5e4_4df4_a0fc_172f5bac0430)
#define h_341a3733_d5e4_4df4_a0fc_172f5bac0430

#include "generator.h"

namespace hammer
{
   class exe_and_shared_lib_generator : public generator
   {
      public:
         exe_and_shared_lib_generator(hammer::engine& e,
                                      const std::string& name,
                                      const consumable_types_t& source_types,
                                      const producable_types_t& target_types,
                                      bool composite,
                                      const feature_set* c = 0);

         virtual std::vector<boost::intrusive_ptr<build_node> >
            construct(const target_type& type_to_construct, 
                      const feature_set& props,
                      const std::vector<boost::intrusive_ptr<build_node> >& sources,
                      const basic_target* t,
                      const pstring* name,
                      const main_target& owner) const;
      private:
         const target_type& searched_lib_;
   };

}

#endif //h_341a3733_d5e4_4df4_a0fc_172f5bac0430
