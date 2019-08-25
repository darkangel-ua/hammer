#pragma once
#include <hammer/core/targets_argument_writer.h>

namespace hammer {

class output_dir_argument_writer : public argument_writer {
   public:
      output_dir_argument_writer(const std::string& name) : argument_writer(name) {}
      argument_writer* clone() const override { return new output_dir_argument_writer(*this); }
      std::vector<feature_ref> valuable_features() const override { return {}; }

   protected:
      void write_impl(std::ostream& output,
                      const build_node& node,
                      const build_environment& environment) const override;
};

}
