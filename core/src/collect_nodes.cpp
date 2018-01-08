#include "stdafx.h"
#include <hammer/core/collect_nodes.h>
#include <hammer/core/generator.h>
#include <hammer/core/basic_build_target.h>

using namespace std;

namespace hammer{

typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
typedef std::set<const build_node*> visited_nodes_t;

void collect_nodes(build_node::sources_t& result, 
                   std::set<const build_node*>& visited_nodes,
                   const std::vector<boost::intrusive_ptr<build_node> >& sources, 
                   const std::vector<const target_type*>& types_to_collect,
                   bool recursive)
{
   nodes_t new_sources;
   visited_nodes_t local_visited_nodes;

   for(nodes_t::const_iterator i = sources.begin(), last = sources.end(); i != last; ++i)
   {
      if (visited_nodes.find(i->get()) == visited_nodes.end())
      {
         local_visited_nodes.insert(i->get());
         for(build_node::sources_t::const_iterator s = (**i).sources_.begin(), s_last = (**i).sources_.end(); s != s_last; ++s)
            new_sources.push_back(s->source_node_);

         new_sources.insert(new_sources.end(), (**i).dependencies_.begin(), (**i).dependencies_.end());
      }
      else
         continue;

      for(build_node::targets_t::const_iterator p = (**i).products_.begin(), p_last = (**i).products_.end(); p != p_last; ++p)
      {
         vector<const target_type*>::const_iterator t = std::find(types_to_collect.begin(), types_to_collect.end(), &(**p).type());
         if (t != types_to_collect.end())
            result.push_back(build_node::source_t(*p, *i));
      }
   }

   if (recursive && !new_sources.empty())
   {
      remove_dups(new_sources);
      visited_nodes.insert(local_visited_nodes.begin(), local_visited_nodes.end());
      collect_nodes(result, visited_nodes, new_sources, types_to_collect, recursive);
   }
}

}
