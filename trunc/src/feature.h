#pragma once
#include "pstring.h"
#include <boost/noncopyable.hpp>
#include "feature_def.h"

namespace hammer
{
   class feature_def;
   class main_target;
   class feature : public boost::noncopyable
   {
      public:
         struct path_data
         {
            const main_target* target_;
         };

         feature(const feature_def* def, const pstring& value);
         const feature_def& def() const { return *def_; }
         const pstring& value() const { return value_; }
         feature_attributes attributes() const { return def_->attributes(); }
         template<typename T>
         const T& get() const { return get_impl(static_cast<const T*>(0)); }
         void set(const path_data& v) { path_data_ = v; }

      private:
         const feature_def* def_;
         pstring value_;
         path_data path_data_;

         const path_data& get_impl(const path_data*) const { return path_data_; }
   };
}