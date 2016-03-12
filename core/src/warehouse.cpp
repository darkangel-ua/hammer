#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/call_resolver.h>
#include <hammer/core/project.h>
#include <boost/unordered_set.hpp>
#include <boost/bind.hpp>

using namespace std;
using boost::unordered_set;

namespace hammer{

static
void walk_over_targets(vector<const warehouse_target*>& result,
                       unordered_set<basic_target*>& visited,
                       const vector<basic_target*>& targets)
{
   for(vector<basic_target*>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
      if (warehouse_target* t = dynamic_cast<warehouse_target*>(*i))
         result.push_back(t);
      else if (main_target* m = dynamic_cast<main_target*>(*i)) {
         if (visited.find(m) == visited.end()) {
            visited.insert(m);
            walk_over_targets(result, visited, m->sources());
            walk_over_targets(result, visited, m->dependencies());
         }
      }
   }
}

vector<const warehouse_target*>
find_all_warehouse_unresolved_targets(const vector<basic_target*>& targets)
{
   vector<const warehouse_target*> result;
   unordered_set<basic_target*> visited;

   walk_over_targets(result, visited, targets);

   return result;
}

static
void warehouse_trap_rule(project* p,
                         pstring& target_name)
{
   auto_ptr<basic_meta_target> trap_target(new warehouse_meta_target(*p, target_name));
   p->add_target(trap_target);
}

void install_warehouse_rules(call_resolver& resolver,
                             engine& engine)
{
   resolver.insert("warehouse-trap", boost::function<void (project*, pstring&)>(boost::bind(warehouse_trap_rule, _1, _2)));
}

}
