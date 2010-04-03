#include "stdafx.h"
#include <hammer/core/cleaner.h>
#include <hammer/core/basic_target.h>
#include <boost/unordered_set.hpp>

namespace hammer{

struct cleaner::impl_t
{
   impl_t(engine& e, 
          const build_environment& environment) 
      : engine_(e),
        environment_(environment)
   {}

   void clean_all(result& r, build_node& n);

   engine& engine_;
   const build_environment& environment_;

   boost::unordered_set<build_node*> visited_nodes_;
};

void cleaner::impl_t::clean_all(result& r, build_node& n)
{
   for(build_node::sources_t::iterator i = n.sources_.begin(), last = n.sources_.end(); i != last; ++i)
      if (visited_nodes_.find(i->source_node_.get()) == visited_nodes_.end())
      {
         visited_nodes_.insert(i->source_node_.get());
         clean_all(r, *i->source_node_);
      }

   for(build_node::targets_t::iterator i = n.products_.begin(), last = n.products_.end(); i != last; ++i)
   {
      (**i).clean(environment_);
      r.cleaned_target_count_++;
   }

   for(build_node::nodes_t::iterator i = n.dependencies_.begin(), last = n.dependencies_.end(); i != last; ++i)
      if (visited_nodes_.find(i->get()) == visited_nodes_.end())
      {
         visited_nodes_.insert(i->get());
         clean_all(r, **i);
      }
}

cleaner::cleaner(engine& e, const build_environment& environment) 
   : impl_(new impl_t(e, environment))
{}

cleaner::result cleaner::clean_all(nodes_t& nodes)
{
   result r;
   impl_->visited_nodes_.clear();
   
   for(nodes_t::iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      impl_->clean_all(r, **i);

   return r;
}

}
