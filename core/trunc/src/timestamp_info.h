#if !defined(h_37ae39f5_cb84_4f4a_8c59_1f25dc0e0bb1)
#define h_37ae39f5_cb84_4f4a_8c59_1f25dc0e0bb1

#include <boost/date_time/posix_time/ptime.hpp>

namespace hammer
{
   class timestamp_info_t
   {
      public:
         enum getter_policy_t {just_get, check, recheck};

         timestamp_info_t() : is_unknown_(true) {}
         timestamp_info_t(const boost::posix_time::ptime& timestamp) 
            : timestamp_(timestamp),
              is_unknown_(false),
              hight_cpu_cost_(false),
              high_io_cost_(false)
         {}

         boost::posix_time::ptime timestamp_;
         bool is_unknown_ : 1;
         bool hight_cpu_cost_ : 1;
         bool high_io_cost_ : 1;
   };
}

#endif //h_37ae39f5_cb84_4f4a_8c59_1f25dc0e0bb1
