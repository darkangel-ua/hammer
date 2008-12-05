#if !defined(h_2d289661_69e9_45eb_a229_665542ce6533)
#define h_2d289661_69e9_45eb_a229_665542ce6533

#include "build_node.h"
#include <boost/date_time/posix_time/ptime.hpp>

namespace hammer
{
   class actuality_checker
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;

         std::size_t check(const nodes_t& nodes);
      
      private: 
         std::size_t check(boost::posix_time::ptime& max_target_time, const build_node& node);
   };
}

#endif //h_2d289661_69e9_45eb_a229_665542ce6533
