#if !defined(h_039e543d_03a7_4517_bda4_4618e0db9794)
#define h_039e543d_03a7_4517_bda4_4618e0db9794

#include "pstring.h"
#include <boost/noncopyable.hpp>
#include "source_decl.h"
#include "feature_attributes.h"
#include "feature_def.h"

namespace hammer
{
   class basic_meta_target;
   class basic_target;

   class feature_base : public boost::noncopyable
   {
      public:
         struct path_data
         {
            const basic_meta_target* target_;
            bool operator < (const path_data& rhs) const { return target_ < rhs.target_; }
            bool operator != (const path_data& rhs) const { return target_ != rhs.target_; } 
         };

         struct dependency_data
         {
            source_decl source_;
            bool operator < (const dependency_data& rhs) const { return source_ < rhs.source_; }
            bool operator != (const dependency_data& rhs) const { return source_ != rhs.source_; }
         };

         struct generated_data
         {
            const basic_target* target_;
            bool operator < (const generated_data& rhs) const { return target_ < rhs.target_; }
            bool operator != (const generated_data& rhs) const { return target_ != rhs.target_; }
         };

         feature_base(const feature_def_base* def, const pstring& value);
         const std::string& name() const { return definition_->name(); }
         const pstring& value() const { return value_; }
         feature_attributes attributes() const { return definition_->attributes(); }
         const path_data& get_path_data() const { return path_data_; }
         path_data& get_path_data() { return path_data_; }
         const dependency_data& get_dependency_data() const { return dependency_data_; }
         dependency_data& get_dependency_data() { return dependency_data_; }
         const generated_data& get_generated_data() const { return generated_data_; }
         generated_data& get_generated_data() { return generated_data_; }
         
      protected:
         const feature_def_base* definition_;
         pstring value_;
         path_data path_data_;
         dependency_data dependency_data_;
         generated_data generated_data_;
   };
}

#endif //h_039e543d_03a7_4517_bda4_4618e0db9794
