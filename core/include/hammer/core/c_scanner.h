#pragma once
#include <vector>
#include <utility>
#include <boost/date_time/posix_time/ptime.hpp>
#include <hammer/core/scaner.h>
#include <hammer/core/hashed_location.h>

namespace hammer {

struct c_scanner_context;

class c_scanner : public scanner {
      friend struct c_scanner_context;

   public:
      c_scanner() : scanner("C scanner") {}

      boost::posix_time::ptime
      process(const basic_build_target& t,
              const hammer::build_node& node,
              scanner_context& context) const override;

      std::shared_ptr<scanner_context>
      create_context(const build_environment& env) const override;

   private:
      mutable std::weak_ptr<scanner_context> context_;
};

}
