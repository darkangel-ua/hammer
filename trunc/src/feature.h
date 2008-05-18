#pragma once
#include "pstring.h"
#include <boost/noncopyable.hpp>
#include "feature_def.h"

namespace hammer
{
   class feature_def;
   class feature : public boost::noncopyable
   {
      public:
         feature(const feature_def* def, const pstring& value);
         const feature_def& def() const { return *def_; }
         const pstring& value() const { return value_; }
         feature_attributes attributes() const { return def_->attributes(); }

      private:
         const feature_def* def_;
         pstring value_;
   };
}