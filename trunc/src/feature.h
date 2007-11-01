#pragma once
#include "pstring.h"
#include <boost/noncopyable.hpp>

namespace hammer
{
   class feature_def;
   class feature : public boost::noncopyable
   {
      public:
         feature(const feature_def* def, const pstring& value);
      
      private:
         const feature_def* def_;
         pstring value_;
   };
}