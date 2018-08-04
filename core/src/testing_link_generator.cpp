#include <hammer/core/types.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/testing_link_generator.h>

namespace hammer {

testing_link_generator::testing_link_generator(engine& e,
                                               std::unique_ptr<generator> link_generator)
   : testing_compile_link_base_generator(e,
                                         "testing.link",
                                         move(link_generator),
                                         make_product_types(e, {types::TESTING_LINK_SUCCESSFUL}),
                                         types::TESTING_LINK_SUCCESSFUL)
{
}

}
