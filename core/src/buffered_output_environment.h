#pragma once
#include <functional>
#include <boost/iostreams/filter/line.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <hammer/core/build_environment.h>
#include <hammer/core/proxied_build_environment.h>

namespace hammer { namespace details {

class buffered_output_environment : public proxied_build_environment {
      struct one_line_pusher : boost::iostreams::line_filter {
            struct category
                  : boost::iostreams::line_filter::category,
                    boost::iostreams::flushable_tag
                {};

            string_type
            do_filter(const string_type& s) override { return s; }

            template<typename Sink>
            bool flush(Sink& snk) {
               return true;
            }
      };

      struct flushable_ostream {
            using char_type = std::ostream::char_type;
            struct category
                  : boost::iostreams::sink_tag,
                    boost::iostreams::flushable_tag
            {};

            flushable_ostream(std::ostream& s) : s_(s) {}

            std::streamsize
            write(const char_type* s,
                  std::streamsize n) {

               if (!s_.get().write(s, n))
                     throw std::runtime_error("[flushable_ostream] Failed to write");

               return n;
            }

            bool flush() {
               return s_.get().flush().good();
            }

            std::reference_wrapper<std::ostream> s_;
      };

   public:
      buffered_output_environment(const hammer::build_environment& env)
         : proxied_build_environment(env)
      {
         // boost iostream adapt std::ostream to something not capable to flush - flush operation is just no-op
         // it cost me a LOT to figure that out :(((
         output_stream_.push(one_line_pusher());
         output_stream_.push(flushable_ostream(env.output_stream()));
         error_stream_.push(one_line_pusher());
         error_stream_.push(flushable_ostream(env.error_stream()));
      }

      std::ostream&
      output_stream() const override { return output_stream_; }

      std::ostream&
      error_stream() const override { return error_stream_; }

   private:
      mutable boost::iostreams::filtering_ostream output_stream_;
      mutable boost::iostreams::filtering_ostream error_stream_;
};

}}
