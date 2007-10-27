#pragma once

#include "../../location.h"

namespace hammer
{
   class feature_set;
   class main_target;

   namespace project_generators
   {
      class msvc_project
      {
         public:
            msvc_project(const main_target* t);
            void add_variant(const main_target* t, 
                             const feature_set* props);

         private:
            const main_target* t_;
      };
   }
}