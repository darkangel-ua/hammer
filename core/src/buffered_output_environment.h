#pragma once
#include <boost/iostreams/filter/line.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <hammer/core/build_environment.h>
#include <hammer/core/proxied_build_environment.h>

namespace hammer { namespace details {

class buffered_output_environment : public proxied_build_environment {
      struct one_line_pusher : boost::iostreams::line_filter {
         private:
            string_type do_filter(const string_type& s) override {
               return s;
            }
      };

   public:
      buffered_output_environment(const hammer::build_environment& env)
         : proxied_build_environment(env)
      {
         output_stream_.push(one_line_pusher());
         output_stream_.push(env.output_stream());
         error_stream_.push(one_line_pusher());
         error_stream_.push(env.error_stream());
      }

      std::ostream& output_stream() const override
      {
         return output_stream_;
      }

      std::ostream& error_stream() const override
      {
         return error_stream_;
      }

   private:
      mutable boost::iostreams::filtering_ostream output_stream_;
      mutable boost::iostreams::filtering_ostream error_stream_;
};

}}
