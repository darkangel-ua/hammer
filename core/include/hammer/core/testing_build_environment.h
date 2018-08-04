#pragma once
#include <hammer/core/proxied_build_environment.h>

namespace hammer {

class testing_build_environment : public proxied_build_environment {
   public:
      testing_build_environment(const build_environment& env) : proxied_build_environment(env) {}

      std::ostream& output_stream() const override { return output_stream_; }
      std::ostream& error_stream() const override { return error_stream_; }

      mutable std::ostringstream output_stream_;
      mutable std::ostringstream error_stream_;
};

}
