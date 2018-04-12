#pragma once

namespace hammer {

class generator_registry;
class engine;

void add_testing_generators(engine& e,
                            generator_registry& gr);

}
