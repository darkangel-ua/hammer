#if !defined(h_655d0ae9_850d_4f01_a912_19b32eaf223e)
#define h_655d0ae9_850d_4f01_a912_19b32eaf223e

#include "pstring.h"
#include <boost/noncopyable.hpp>
#include "feature_def.h"
#include "source_decl.h"

namespace hammer
{
   class feature_def;
   class basic_meta_target;
   class basic_target;

   //FIXME: Optimize data handling
   class feature : public boost::noncopyable
   {
      public:
         struct path_data
         {
            const basic_meta_target* target_;
         };

         struct dependency_data
         {
            source_decl source_;
         };

         struct generated_data
         {
            const basic_target* target_;
         };

         feature(const feature_def* def, const pstring& value);
         const feature_def& def() const { return *def_; }
         const std::string& name() const { return def().name(); }
         const pstring& value() const { return value_; }
         feature_attributes attributes() const { return def_->attributes(); }
         const path_data& get_path_data() const { return path_data_; }
         path_data& get_path_data() { return path_data_; }
         const dependency_data& get_dependency_data() const { return dependency_data_; }
         dependency_data& get_dependency_data() { return dependency_data_; }
         const generated_data& get_generated_data() const { return generated_data_; }
         generated_data& get_generated_data() { return generated_data_; }
         bool operator == (const feature& rhs) const;

      private:
         const feature_def* def_;
         pstring value_;
         path_data path_data_;
         dependency_data dependency_data_;
         generated_data generated_data_;
   };
}

#endif //h_655d0ae9_850d_4f01_a912_19b32eaf223e
