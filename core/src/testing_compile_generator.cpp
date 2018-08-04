#include <hammer/core/types.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/testing_compile_generator.h>

namespace hammer {

testing_compile_generator::testing_compile_generator(engine& e,
                                                     std::unique_ptr<generator> compile_generator)
   : testing_compile_link_base_generator(e,
                                         "testing.compile",
                                         move(compile_generator),
                                         make_product_types(e, {types::TESTING_COMPILE_SUCCESSFUL}),
                                         types::TESTING_COMPILE_SUCCESSFUL)
{
}

}
