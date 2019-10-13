#pragma once
#include <hammer/core/generated_build_target.h>
#include <hammer/core/mkdir_action.h>

namespace hammer {

class directory_build_target : public generated_build_target {
   public:
      directory_build_target(const main_target* mt,
                             const location_t& dir);
      const std::shared_ptr<mkdir_action>& action() const { return action_; }
      const location_t& location() const override { return dir_to_create_; }
      const std::string& hash() const override;
      bool clean(const build_environment& environment) const override;

   private:
      location_t dir_to_create_;
      std::shared_ptr<mkdir_action> action_;

      void timestamp_info_impl() const override;
};

}
