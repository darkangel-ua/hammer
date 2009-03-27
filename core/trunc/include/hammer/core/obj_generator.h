#if !defined(h_65ecc706_d1d8_4208_a8ce_4e372daffafa)
#define h_65ecc706_d1d8_4208_a8ce_4e372daffafa

#include "generator.h"

namespace hammer
{
   class obj_generator : public generator
   {
      public:
         obj_generator(hammer::engine& e,
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
                      const pstring* composite_target_name,
                      const main_target& owner) const;
      private:
         const target_type& obj_type_;
   };
}

#endif //h_65ecc706_d1d8_4208_a8ce_4e372daffafa
