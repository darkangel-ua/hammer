#pragma once
#include <boost/make_unique.hpp>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <hammer/core/proxied_build_environment.h>

namespace hammer {

class testing_build_environment : public proxied_build_environment {
   public:
      testing_build_environment(const build_environment& env,
                                const bool tee = false)
         : proxied_build_environment(env),
           tee_(tee)
      {
         namespace io = boost::iostreams;
         if (tee) {
            output_tee_stream_ = boost::make_unique<io::filtering_ostream>(io::tee(env.output_stream(), output_stream_));
            error_tee_stream_ = boost::make_unique<io::filtering_ostream>(io::tee(env.error_stream(), error_stream_));
         }
      }

      std::ostream&
      output_stream() const override
      {
         return tee_ ? static_cast<std::ostream&>(*output_tee_stream_) : output_stream_;
      }

      std::ostream&
      error_stream() const override {
         return tee_ ? static_cast<std::ostream&>(*error_tee_stream_) : error_stream_;
      }

      const bool tee_;
      mutable std::ostringstream output_stream_;
      mutable std::ostringstream error_stream_;
      std::unique_ptr<boost::iostreams::filtering_ostream> output_tee_stream_;
      std::unique_ptr<boost::iostreams::filtering_ostream> error_tee_stream_;
};

}
