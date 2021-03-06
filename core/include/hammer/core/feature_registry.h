#pragma once
#include <map>
#include <functional>
#include <hammer/core/feature_def.h>
#include <hammer/core/feature_value_ns_fwd.h>
#include <hammer/core/feature_ref.h>

namespace hammer {

class feature_set;
class feature;
class pool;
class project;
class source_decl;
class basic_target;

class feature_registry {
   public:
      struct impl_t;
      feature_registry(std::shared_ptr<feature_registry> parent = {});
      ~feature_registry();

      // add to THIS registry
      feature_def&
      add_feature_def(const std::string& name,
                      const feature_def::legal_values_t& legal_values = {},
                      feature_attributes attributes = {});

      // search over hierarchy
      feature_def&
      get_def(const std::string& name);
      const feature_def&
      get_def(const std::string& name) const;

      // search over hierarchy
      const feature_def*
      find_def(const std::string& name) const;
      feature_def*
      find_def(const std::string& name);

      // search over hierarchy
      const feature_def*
      find_def(const char* feature_name) const;

      feature_set*
      make_set() const;

      // feature will belongs to registry that owns definition
      feature_ref
      create_feature(const std::string& name,
                     const std::string& value) const;
      // path feature
      feature_ref
      create_feature(const std::string& name,
                     const std::string& value,
                     const project& p) const;
      // dependency feature
      feature_ref
      create_feature(const std::string& name,
                     const source_decl& s) const;
      // generated feature
      feature_ref
      create_feature(const std::string& name,
                     const std::string& value,
                     const basic_target& t) const;

      feature_ref
      create_feature(const feature& f,
                     const std::string& subfeature_name,
                     const std::string& subfeature_value) const;

      // get over over hierarchy
      // create only in THIS
      const feature_value_ns_ptr&
      get_or_create_feature_value_ns(const std::string& ns);

      feature_set&
      add_defaults(feature_set& s) const;

      // find definition for complex names such as os-version where version is subfeature
      // search over hierarchy
      const feature_def*
      find_def_from_full_name(const char* feature_name) const;

   private:
      std::unique_ptr<impl_t> impl_;

      feature*
      simply_create_feature(const feature_def& def,
                            const std::string& value) const;
      feature*
      simply_create_feature(const feature_def& def,
                            const std::string& value,
                            const std::function<feature*(const feature_def& def)>& constructor) const;
      feature*
      create_feature(const std::string& name,
                     const std::string& value,
                     const std::function<feature*(const feature_def& def)>& constructor) const;
};

}
