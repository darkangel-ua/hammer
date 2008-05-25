#pragma once
#include "pstring.h"
#include <boost/noncopyable.hpp>
#include "feature_def.h"

namespace hammer
{
   class feature_def;
   class basic_meta_target;
   class feature : public boost::noncopyable
   {
      public:
         struct path_data
         {
            const basic_meta_target* target_;
         };

         feature(const feature_def* def, const pstring& value);
         const feature_def& def() const { return *def_; }
         const std::string& name() const { return def().name(); }
         const pstring& value() const { return value_; }
         feature_attributes attributes() const { return def_->attributes(); }
         const path_data& get_path_data() const { return path_data_; }
         path_data& get_path_data() { return path_data_; }
         size_t composite_size() const { return def().composite_size(value().to_string()); }

      private:
         const feature_def* def_;
         pstring value_;
         path_data path_data_;
   };
}