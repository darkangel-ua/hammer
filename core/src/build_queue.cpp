#include <boost/unordered_map.hpp>
#include <hammer/core/main_target.h>
#include "build_queue.h"

namespace hammer { namespace details {

using nodes_to_build_t = boost::unordered_map<const build_node*, build_queue_node_t*>;

struct gather_ctx {
   const project* bounds_;
   const bool unconditional_build_;
   boost::object_pool<build_queue_node_t>& pool_;
   nodes_to_build_t nodes_to_build_;
};

static
build_queue_node_t&
gather_nodes(gather_ctx& ctx,
             build_node& node);

static
build_queue_node_t&
gather_nodes(gather_ctx& ctx,
             build_queue_node_t& parent_node,
             build_node& node)
{
   auto a = ctx.nodes_to_build_.find(&node);
   if (a == ctx.nodes_to_build_.end()) {
      ++parent_node.dependencies_count_;
      build_queue_node_t& result = gather_nodes(ctx, node);
      result.uses_nodes_.insert(&parent_node);

      return result;
   } else {
      if (a->second->uses_nodes_.insert(&parent_node).second)
         ++parent_node.dependencies_count_;

      return *a->second;
   }
}

static
build_queue_node_t&
gather_nodes(gather_ctx& ctx,
             build_node& node)
{
   build_queue_node_t& ctx_node = *ctx.pool_.construct(&node);
   ctx.nodes_to_build_.insert(std::make_pair(&node, &ctx_node));

   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      if (!i->source_node_->up_to_date() || ctx.unconditional_build_)
         if (ctx.bounds_ == nullptr || &i->source_node_->products_owner().get_project() == ctx.bounds_) // we don't build nodes that don't belongs to bounds
            gather_nodes(ctx, ctx_node, *i->source_node_);

   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      if (!(**i).up_to_date() || ctx.unconditional_build_)
         if (ctx.bounds_ == nullptr || &(**i).products_owner().get_project() == ctx.bounds_) // we don't build nodes that don't belongs to bounds
            gather_nodes(ctx, ctx_node, **i);

   return ctx_node;
}

build_queue::build_queue(build_nodes_t& nodes,
                         const bool unconditional_build,
                         const hammer::project* bounds)
{
   gather_ctx ctx = { bounds, unconditional_build, pool_ };

   for (auto i : nodes) {
      if (!i->up_to_date() || unconditional_build)
         gather_nodes(ctx, *i);
   }

   for (auto np : ctx.nodes_to_build_)
      insert(np.second);
}

build_queue::~build_queue()
{}

}}

