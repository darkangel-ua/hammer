#pragma once
#include "generator.h"

namespace hammer
{
   class basic_target;
   class main_target;

   class generator_registry
   {
      public:
         typedef std::map<std::string, generator> generators_t;

         void insert(const generator& g);
         std::auto_ptr<build_node> construct(main_target* mt) const;

      private:
         generators_t generators_;

         std::vector<const generator*> find_viable_generators(const type& t) const;
   };
}