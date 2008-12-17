#include "stdafx.h"
#include "builder.h"
#include "build_action.h"

namespace hammer{

builder::builder(const build_environment& environment) : environment_(environment)
{
}


void builder::build(const build_node& node)
{
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      if (!(**i).up_to_date())
         build(**i);

   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      if (!i->source_node_->up_to_date())
         build(*i->source_node_);

   if (const build_action* action = node.action())
      action->execute(node, environment_);
}

void builder::build(const nodes_t& nodes)
{
   for(nodes_t::const_iterator i = nodes.begin(), last= nodes.end(); i != last; ++i)
      build(**i);
}

}
