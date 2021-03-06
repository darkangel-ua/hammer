#pragma once
#include <string>
#include <vector>
#include <hammer/core/build_action_fwd.h>
#include <hammer/core/feature_ref.h>

namespace hammer {

class build_node;
class build_environment;
class basic_target;
class feature;

class build_action {
   public:
      build_action(const std::string& name) : name_(name) {}
      const std::string& name() const { return name_; }

      bool execute(const build_node& node,
                   const build_environment& environment,
                   const bool expected_to_fail = false) const;
      virtual
      std::string
      target_tag(const build_node& node,
                 const build_environment& environment) const = 0;

      // FIXME: this need to factored out. This should be fully const object
      void batched_action(const build_action_ptr& ba) { batched_action_ = ba; }
      const build_action_ptr& batched_action() const { return batched_action_; }
      virtual std::vector<feature_ref> valuable_features() const = 0;

      virtual ~build_action();

   protected:
      virtual
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const = 0;
      virtual
      void clean_on_fail(const build_node& node,
                         const build_environment& environment) const;

   private:
      std::string name_;
      build_action_ptr batched_action_;
};

}
