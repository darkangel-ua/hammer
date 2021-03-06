#pragma once
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/unordered_map.hpp>
#include <hammer/core/build_node.h>

namespace hammer {

class target_type;
class scanner_context;
class scanner;
class engine;
class build_environment;

class actuality_checker {
   public:
      typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
      actuality_checker(const engine& e, const build_environment& env) : engine_(e), env_(env) {}
      std::size_t check(nodes_t& nodes);

   private:
      typedef boost::unordered_map<const target_type*, std::shared_ptr<scanner_context> > scanner_contexts_t;

      scanner_contexts_t scanner_contexts_;
      const engine& engine_;
      const build_environment& env_;

      // returns 'true' if node needs to be updated
      bool check(boost::posix_time::ptime& max_target_time, std::size_t& nodes_to_update, build_node& node);
      scanner_context& get_scanner_context(const target_type& t, const scanner& s);
      void mark_to_update(build_node& node,
                          std::size_t& nodes_to_update,
                          const main_target& products_owner);
};

void mark_to_update(build_nodes_t& nodes);

}
