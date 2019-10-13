#pragma once
#include <hammer/core/feature_attributes.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/source_decl.h>

namespace hammer {

class subfeature;
class feature_registry;
class basic_meta_target;
class basic_target;
class project;

// managed by feature_registry
class feature {
   public:
      friend class feature_registry;
      typedef std::vector<const subfeature*> subfeatures_t;

      struct path_data {
         const project* project_;
         bool operator < (const path_data& rhs) const { return project_ < rhs.project_; }
         bool operator != (const path_data& rhs) const { return project_ != rhs.project_; }
      };

      struct dependency_data {
         source_decl source_;
         bool operator < (const dependency_data& rhs) const { return source_ < rhs.source_; }
         bool operator != (const dependency_data& rhs) const { return source_ != rhs.source_; }
      };

      struct generated_data {
         const basic_target* target_;
         bool operator < (const generated_data& rhs) const { return target_ < rhs.target_; }
         bool operator != (const generated_data& rhs) const { return target_ != rhs.target_; }
      };

      const feature_def&
      definition() const { return definition_; }

      const std::string&
      name() const { return definition_.name(); }

      const std::string&
      value() const { return value_; }

      const feature_value_ns_ptr&
      get_value_ns() const;

      feature_attributes
      attributes() const { return definition_.attributes(); }

      const path_data&
      get_path_data() const { return path_data_; }

      const dependency_data&
      get_dependency_data() const { return dependency_data_; }

      const generated_data&
      get_generated_data() const { return generated_data_; }

      // FIXME: will not work when rhs and lhs from different feature_registries
      const subfeature*
      find_subfeature(const subfeature& v) const;

      const subfeature*
      find_subfeature(const std::string& v) const;

      const subfeatures_t&
      subfeatures() const { return subfeatures_; }

      // looking for position of value in definition().legal_values()
      std::size_t
      value_index() const;

      // <toolset>gcc will be equal only to <toolset>gcc - full match, including subfeatures
      bool operator == (const feature& rhs) const;

      // if this == <toolset>gcc-6 then it will contain f == <toolset>gcc AND f == <toolset>gcc-6, partial match
      bool contains(const feature& f) const;

      bool operator != (const feature& rhs) const { return !(*this == rhs); }
      bool operator < (const feature& rhs) const;

   private:
      const feature_def& definition_;
      std::string value_;
      path_data path_data_;
      dependency_data dependency_data_;
      generated_data generated_data_;
      subfeatures_t subfeatures_;

      feature(const feature_def& def,
              std::string value,
              subfeatures_t subfeatures = {});

      // path feature - always has project it belongs to
      feature(const feature_def& def,
              std::string value,
              const project& p);

      // depedency feature always has source as value
      feature(const feature_def& def,
              source_decl s);

      // generated feature always has generated target as value
      feature(const feature_def& def,
              std::string value,
              const basic_target& generated_target);

      ~feature() = default;

      bool equal_without_subfeatures(const feature& rhs) const;
};

}
