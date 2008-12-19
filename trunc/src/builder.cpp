#include "stdafx.h"
#include "builder.h"
#include "build_action.h"

namespace hammer{

builder::builder(const build_environment& environment) : environment_(environment)
{
}


bool builder::build(const build_node& node)
{
   typedef std::vector<const build_node*> not_builded_nodes_t;
   not_builded_nodes_t not_builded_nodes;
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      if (!(**i).up_to_date())
         if (!build(**i))
            not_builded_nodes.push_back(i->get());

   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      if (!i->source_node_->up_to_date())
         if (!build(*i->source_node_))
            not_builded_nodes.push_back(i->source_node_.get());

   if (!not_builded_nodes.empty() && 
       node.action() != NULL)
   {
      std::cout << "...skipped " << node.action()->target_tag(node, environment_) << '\n';
      for(not_builded_nodes_t::const_iterator i = not_builded_nodes.begin(), last = not_builded_nodes.end(); i != last; ++i)
         std::cout << "......for lack of " 
                   << ((**i).action() != NULL ? (**i).action()->target_tag(**i, environment_) 
                                              : "?unknown?") 
                   << '\n';
      return false;
   }

   if (!node.up_to_date())
      if (const build_action* action = node.action())
         return action->execute(node, environment_);
   
   return true;
}

void builder::build(const nodes_t& nodes)
{
   for(nodes_t::const_iterator i = nodes.begin(), last= nodes.end(); i != last; ++i)
      build(**i);
}

}
