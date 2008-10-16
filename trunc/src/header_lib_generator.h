#if !defined(h_0fd8e948_a41d_4666_a0ec_f5eb11e54c43)
#define h_0fd8e948_a41d_4666_a0ec_f5eb11e54c43

#include "generator.h"

namespace hammer
{
   class header_lib_generator : public generator
   {
      public:
         header_lib_generator(hammer::engine& e,
                              const std::string& name,
                              const consumable_types_t& source_types,
                              const producable_types_t& target_types,
                              const feature_set* c = 0);
         virtual std::vector<boost::intrusive_ptr<build_node> >
            construct(const type& target_type, 
                      const feature_set& props,
                      const std::vector<boost::intrusive_ptr<build_node> >& sources,
                      const basic_target* t,
                      const pstring* composite_target_name,
                      const main_target& owner) const;
      private:
         const type& header_type_;
   };
}

#endif //h_0fd8e948_a41d_4666_a0ec_f5eb11e54c43
