#pragma once

namespace hammer
{
   class generator_registry;
   class engine;

   void add_msvc_generators(engine& e, generator_registry& gr);
}