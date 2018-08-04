#include <hammer/core/types.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/testing_link_fail_generator.h>

namespace hammer {

testing_link_fail_generator::testing_link_fail_generator(engine& e,
                                                         std::unique_ptr<generator> failing_link_generator)
   : testing_compile_link_base_generator(e,
                                         "testing.link-fail",
                                         std::move(failing_link_generator),
                                         make_product_types(e, {types::TESTING_OUTPUT, types::TESTING_LINK_FAIL}),
                                         types::TESTING_LINK_FAIL)
{
}

}
