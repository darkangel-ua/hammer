#include "stdafx.h"
#include "shared_lib_dirs_writer.h"
#include "collect_nodes.h"
#include "basic_target.h"

namespace hammer{

void shared_lib_dirs_writer::write_impl(std::ostream& output, 
                                        const build_node& node, 
                                        const build_environment& environment) const
{
   build_node::sources_t collected_nodes;
   std::set<const build_node*> visited_nodes;
   std::vector<const hammer::type*> types_to_collect(1, &shared_lib_type_);
   // FIXME: const problems...
   std::vector<boost::intrusive_ptr<build_node> > sources(1, boost::intrusive_ptr<build_node>(const_cast<build_node*>(&node)));
 
   collect_nodes(collected_nodes, visited_nodes, sources, types_to_collect, true);

   bool is_first = true; 
   for(build_node::sources_t::const_iterator i = collected_nodes.begin(), last = collected_nodes.end(); i != last; ++i)
   {
      if (is_first)
         is_first = false;
      else
         output << ';';

      output << i->source_target_->location();
   }
}

}
