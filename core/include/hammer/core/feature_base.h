#pragma once
#include <boost/noncopyable.hpp>
#include <hammer/core/source_decl.h>
#include <hammer/core/feature_attributes.h>
#include <hammer/core/feature_def.h>

namespace hammer {

class basic_meta_target;
class basic_target;
class project;

class feature_base : public boost::noncopyable {
   public:
      struct path_data
      {
         const project* project_ = nullptr;
         bool operator < (const path_data& rhs) const { return project_ < rhs.project_; }
         bool operator != (const path_data& rhs) const { return project_ != rhs.project_; }
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

      feature_base(const feature_def* def, const std::string& value);
      const std::string& name() const { return definition_->name(); }
      const std::string& value() const { return value_; }

      const feature_value_ns_ptr&
      get_value_ns() const;

      feature_attributes attributes() const { return definition_->attributes(); }
      const path_data& get_path_data() const { return path_data_; }
      path_data& get_path_data() { return path_data_; }

      const dependency_data& get_dependency_data() const { return dependency_data_; }
      // dependency data ALWAYS has target relative to which source_decl.target_path was calculated
      void set_dependency_data(const source_decl& sd,
                               const project* relative_to_project)
      {
         dependency_data_.source_ = sd;
         path_data_.project_ = relative_to_project;
      }

      const generated_data& get_generated_data() const { return generated_data_; }
      generated_data& get_generated_data() { return generated_data_; }

   protected:
      const feature_def* definition_;
      std::string value_;
      path_data path_data_;
      dependency_data dependency_data_;
      generated_data generated_data_;
};

}
