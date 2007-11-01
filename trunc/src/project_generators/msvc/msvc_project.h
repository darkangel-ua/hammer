#pragma once

#include "../../location.h"
#include <vector>
#include <utility>
#include "../../feature_set.h"

namespace hammer
{
   class feature_set;
   class main_target;

   namespace project_generators
   {
      class msvc_project
      {
         public:
            void add_variant(const main_target* t, 
                             const feature_set& props);
            void generate();

         private:
            typedef std::vector<std::pair<const main_target*, const feature_set*> > variants_t;
            variants_t variants_;
      };
   }
}