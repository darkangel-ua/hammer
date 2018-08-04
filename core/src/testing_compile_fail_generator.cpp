#include <hammer/core/types.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/testing_compile_fail_generator.h>

namespace hammer {

testing_compile_fail_generator::testing_compile_fail_generator(engine& e,
                                                               std::unique_ptr<generator> failing_compile_generator)
   : testing_compile_link_base_generator(e,
                                         "testing.compile-fail",
                                         std::move(failing_compile_generator),
                                         make_product_types(e, {types::TESTING_OUTPUT, types::TESTING_COMPILE_FAIL}),
                                         types::TESTING_COMPILE_FAIL)
{
}

}

