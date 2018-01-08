#if !defined(h_65ecc706_d1d8_4208_a8ce_4e372daffafa)
#define h_65ecc706_d1d8_4208_a8ce_4e372daffafa

#include "generator.h"

namespace hammer
{

// this is generator for obj meta target. Consume all input and transfer only obj types to result products
class obj_generator : public generator
{
   public:
      obj_generator(hammer::engine& e);

      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                const basic_build_target* t,
                const std::string* composite_target_name,
                const main_target& owner) const override;
   private:
      const target_type& obj_type_;
};

}

#endif //h_65ecc706_d1d8_4208_a8ce_4e372daffafa
