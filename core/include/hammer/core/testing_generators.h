#pragma once
#include <memory>
#include <functional>
#include <hammer/core/build_action_fwd.h>

namespace hammer {

class generator_registry;
class generator;
class engine;

void add_testing_generators(engine& e,
                            generator_registry& gr);

// will add compile + testing.compile + testing.compile-fail generators
void add_compile_generators(engine& e,
                            const build_action_ptr& compile_action,
                            std::function<std::unique_ptr<generator>(const build_action_ptr& action)> compile_generator_creator);

// will add link + testing.link + testing.link-fail generators
void add_link_generators(engine& e,
                         const build_action_ptr& link_action,
                         std::function<std::unique_ptr<generator>(const build_action_ptr& action)> link_generator_creator);

}
