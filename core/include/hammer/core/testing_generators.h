#if !defined(h_124ed403_f92d_4b1d_986c_9b005ccad2b8)
#define h_124ed403_f92d_4b1d_986c_9b005ccad2b8

#include <memory>

namespace hammer
{
   class generator_registry;
   class engine;
   class generator;
   class build_action;

   void add_testing_generators(engine& e, generator_registry& gr);

   void add_compile_fail_generator(engine& e,
                                   std::unique_ptr<generator> compile_generator,
                                   std::unique_ptr<build_action> compile_action);
}

#endif //h_124ed403_f92d_4b1d_986c_9b005ccad2b8
