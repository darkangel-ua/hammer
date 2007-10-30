#pragma once
#include "pstring.h"

namespace hammer
{
   class feature_def;
   class feature
   {
      public:
         feature(const feature_def* def, const pstring& value);
      
      private:
         const feature_def* def_;
         pstring value_;
   };
}