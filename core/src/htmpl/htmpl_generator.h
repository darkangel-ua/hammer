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
	protected:
		basic_build_target*
		create_target(const main_target* mt,
		              const build_node::sources_t& sources,
		              const std::string* composite_target_name,
		              const produced_type& type,
		              const feature_set* f) const override;
};

}

#endif
