#pragma once
#include <boost/date_time/posix_time/ptime.hpp>

namespace hammer {

class timestamp_info_t {
   public:
      timestamp_info_t() : is_unknown_(true) {}
      timestamp_info_t(const boost::posix_time::ptime& timestamp)
         : timestamp_(timestamp),
           is_unknown_(false)
      {}

      boost::posix_time::ptime timestamp_;
      bool is_unknown_ : 1;
};

}
