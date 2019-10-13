#pragma once
#include <vector>
#include <hammer/core/argument_writer.h>

namespace hammer {

class target_type;

class targets_argument_writer : public argument_writer {
   public:
      targets_argument_writer(const std::string& name, const target_type& t);
      const target_type& source_type() const { return *source_type_; }
      std::vector<feature_ref> valuable_features() const override { return {}; }

   private:
      const target_type* source_type_;
};

}
