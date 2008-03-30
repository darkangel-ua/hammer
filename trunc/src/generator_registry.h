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
         std::vector<basic_target*> transform(main_target* mt) const;

      private:
         generators_t generators_;

         std::vector<basic_target*> transform_impl();
   };
}