#include "stdafx.h"
#include <hammer/core/generic_batcher.h>
#include <hammer/core/build_action.h>
#include <boost/unordered_map.hpp>
#include <vector>

using namespace std;

namespace hammer{

void generic_batcher::process(const build_node::nodes_t& nodes)
{
   generic_batcher batcher;
   for(build_node::nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      if (batcher.visited_nodes_.find(i->get()) == batcher.visited_nodes_.end() &&
         (**i).up_to_date() != boost::tribool::true_value)
      {
         batcher.process_impl(**i);
      }
}

void generic_batcher::process_impl(build_node& node) const
{
   visited_nodes_.insert(&node);

   typedef boost::unordered_map<pair<const feature_set*, const build_action*>, build_node::sources_t> selected_nodes_t;

   // splitting sources for batchable and unbatchable
   selected_nodes_t selected_sources;
   build_node::sources_t unbatchable_sources;
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      if (!i->source_node_->is_composite() &&
          i->source_node_->sources_.size() == 1 &&
          i->source_node_->products_.size() == 1 &&
          i->source_node_->action() != NULL &&
          i->source_node_->action()->batched_action() != NULL &&
          i->source_node_->up_to_date() != boost::tribool::true_value)
      {
         selected_sources[make_pair(&i->source_node_->build_request(), 
                                    i->source_node_->action())].push_back(*i);
      }
      else
         unbatchable_sources.push_back(*i);
   }

   for(selected_nodes_t::iterator i = selected_sources.begin(); i != selected_sources.end();)
   {
      if (i->second.size() == 1)
      {
         unbatchable_sources.push_back(i->second.front());
         i = selected_sources.erase(i);
      }
      else
         ++i;
   }

   if (!selected_sources.empty())
   {
      // now we have some batchable sources. Let's batch it!
      node.down_.clear();
      node.sources_.clear();
      for(selected_nodes_t::const_iterator i = selected_sources.begin(); i != selected_sources.end(); ++i)
      {
         const build_node& front_node = *i->second.front().source_node_;
         boost::intrusive_ptr<build_node> new_node(new build_node(front_node.products_owner(), false));
         new_node->up_to_date(boost::tribool::false_value);
         new_node->action(front_node.action()->batched_action());
         new_node->targeting_type_ = front_node.targeting_type_;
         
         for(build_node::sources_t::const_iterator n = i->second.begin(), n_last = i->second.end(); n != n_last; ++n)
         {
            new_node->sources_.insert(new_node->sources_.end(), n->source_node_->sources_.begin(), n->source_node_->sources_.end());
            new_node->products_.insert(new_node->products_.end(), n->source_node_->products_.begin(), n->source_node_->products_.end());
            new_node->dependencies_.insert(new_node->dependencies_.end(), n->source_node_->dependencies_.begin(), n->source_node_->dependencies_.end());
            new_node->down_.insert(new_node->down_.end(), n->source_node_->down_.begin(), n->source_node_->down_.end());
         }
         
         for(build_node::targets_t::const_iterator s = new_node->products_.begin(), s_last = new_node->products_.end(); s != s_last; ++s)
            node.sources_.push_back(build_node::source_t(*s, new_node));

         node.down_.push_back(new_node);
      }

      for(build_node::sources_t::const_iterator i = unbatchable_sources.begin(), last = unbatchable_sources.end(); i != last; ++i)
      {
         node.sources_.push_back(*i);
         node.down_.push_back(i->source_node_);
      }

      sort(node.down_.begin(), node.down_.end());
      node.down_.erase(unique(node.down_.begin(), node.down_.end()), node.down_.end());
   }


   for(build_node::nodes_t::const_iterator i = node.down_.begin(), last = node.down_.end(); i != last; ++i)
   {
      if (visited_nodes_.find(i->get()) == visited_nodes_.end() &&
         (**i).up_to_date() != boost::tribool::true_value)
      {
         process_impl(**i);
      }
   }

   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
   {
      if (visited_nodes_.find(i->get()) == visited_nodes_.end() &&
          (**i).up_to_date() != boost::tribool::true_value)
      {
         process_impl(**i);
      }
   }
}

}

