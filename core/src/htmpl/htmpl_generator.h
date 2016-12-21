#ifndef HAMMER_CORE_HTMPL_HTMPL_GENERATOR
#define HAMMER_CORE_HTMPL_HTMPL_GENERATOR

#include <hammer/core/generator.h>

namespace hammer {

class htmpl_generator : public generator
{
   public:
      htmpl_generator(engine& e,
                      const std::string& name,
                      const type_tag& product_type);

};

}

#endif
