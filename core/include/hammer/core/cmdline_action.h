#pragma once
#include <vector>
#include <hammer/core/build_action.h>
#include <hammer/core/cmdline_builder.h>

namespace hammer {

class cmdline_action : public build_action {
   public:
      template<typename T>
      cmdline_action(const std::string& name,
                     const std::shared_ptr<T>& target_writer)
         : build_action(name),
           target_writer_(std::static_pointer_cast<argument_writer>(target_writer))
      {
      }

      template<typename T>
      cmdline_action(const std::string& name,
                     std::shared_ptr<T>& target_writer,
                     const cmdline_builder& rsp_builder)
      : build_action(name),
        target_writer_(std::static_pointer_cast<argument_writer>(target_writer)),
        rsp_builder_(new cmdline_builder(rsp_builder))
      {
      }

      std::string
      target_tag(const build_node& node,
                 const build_environment& environment) const override;

      std::vector<feature_ref>
      valuable_features() const override;

      cmdline_action& operator +=(const cmdline_builder& b);

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;
      virtual
      bool run_shell_commands(const std::vector<std::string>& commands,
                              const build_node& node,
                              const build_environment& environment) const;

   private:
      typedef std::vector<cmdline_builder> builders_t;

      builders_t builders_;
      std::shared_ptr<argument_writer> target_writer_;
      std::unique_ptr<cmdline_builder> rsp_builder_;
};

}
