#pragma once
#include "generator.h"

namespace hammer
{
   class basic_target;

   class generator_registry
   {
      public:
         typedef std::map<std::string, generator> generators_t;

         void insert(const generator& g);
         std::vector<basic_target*> transform(const std::vector<basic_target*>& targets) const;

      private:
         generators_t generators_;

         std::vector<basic_target*> transform_impl();
   };
}