#if !defined(h_6db05e98_3b97_4047_ba6a_1abe19756798)
#define h_6db05e98_3b97_4047_ba6a_1abe19756798

#include "generator.h"

namespace hammer
{
   class copy_generator : public generator
   {
      public:
         copy_generator(hammer::engine& e);

         virtual build_nodes_t
         construct(const target_type& type_to_construct, 
                   const feature_set& props,
                   const std::vector<boost::intrusive_ptr<build_node> >& sources,
                   const basic_target* t,
                   const pstring* composite_target_name,
                   const main_target& owner) const;
   };
}

#endif //h_6db05e98_3b97_4047_ba6a_1abe19756798
