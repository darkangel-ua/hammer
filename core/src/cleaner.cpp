#include <boost/unordered_set.hpp>
#include <hammer/core/basic_build_target.h>
#include "build_queue.h"
#include <hammer/core/cleaner.h>

namespace hammer {

cleaner::result
cleaner::clean_all(const build_environment& environment,
                   build_nodes_t& nodes)
{
   result r;

   details::build_queue build_queue{nodes, true, nullptr};

   for (auto i = build_queue.get<0>().rbegin(), last = build_queue.get<0>().rend(); i != last; ++i) {
      build_node& node = *(**i).node_;
      for (const auto& t : node.products_)
         r.cleaned_target_count_ += t->clean(environment);
   }

   return r;
}

}
