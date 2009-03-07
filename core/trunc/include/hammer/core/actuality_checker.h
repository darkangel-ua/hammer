#if !defined(h_2d289661_69e9_45eb_a229_665542ce6533)
#define h_2d289661_69e9_45eb_a229_665542ce6533

#include "build_node.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/unordered_map.hpp>

namespace hammer
{
   class type;
   class scanner_context;
   class scanner;
   class engine;
   class build_environment;

   class actuality_checker
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
         actuality_checker(const engine& e, const build_environment& env) : engine_(e), env_(env) {}
         std::size_t check(nodes_t& nodes);
      
      private: 
         typedef boost::unordered_map<const type*, boost::shared_ptr<scanner_context> > scanner_contexts_t;
         
         scanner_contexts_t scanner_contexts_;
         const engine& engine_;
         const build_environment& env_;

         std::size_t check(boost::posix_time::ptime& max_target_time, build_node& node);
         scanner_context& get_scanner_context(const type& t, const scanner& s);
   };
}

#endif //h_2d289661_69e9_45eb_a229_665542ce6533
