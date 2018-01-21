#include "stdafx.h"
#include <sstream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/thread/thread.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/format.hpp>
#include <hammer/core/builder.h>
#include <hammer/core/build_action.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/main_target.h>
#include "buffered_output_environment.h"

using namespace boost;
using namespace boost::multi_index;
using std::string;
using std::stringstream;

namespace hammer{
namespace{

struct build_queue_node_t;
typedef unordered_set<build_queue_node_t*> build_queue_nodes_t;

struct build_queue_node_t
{
   build_queue_node_t(build_node* node) 
      : node_(node),
        dependencies_count_(0), 
        some_dependencies_failed_to_build_(false) 
   {}

   build_node* node_;
   unsigned int dependencies_count_;
   bool some_dependencies_failed_to_build_;
   build_queue_nodes_t uses_nodes_;
};

typedef unordered_map<const build_node*, build_queue_node_t*> nodes_to_build_t;
struct build_queue_dependency_key_extractor
{
   typedef unsigned int result_type;
   result_type operator()(const build_queue_node_t* v) const { return v->dependencies_count_; }
};

struct build_queue_node_key_extractor
{
   typedef build_node* result_type;
   result_type operator()(const build_queue_node_t* v) const { return v->node_; }
};

struct dependency_decrementor
{
   void operator()(build_queue_node_t* v) { --v->dependencies_count_; }
};

typedef multi_index_container<build_queue_node_t*, 
                              indexed_by<ordered_non_unique<build_queue_dependency_key_extractor>,
                                         hashed_unique<identity<build_queue_node_t*> > >
                             > build_queue_t;
typedef build_queue_t::nth_index<0>::type queue_index_t;

typedef unordered_set<build_queue_node_t*> nodes_in_progress_t;

struct worker_ctx_t
{
   worker_ctx_t(asio::io_service& scheduler,
                asio::io_service::strand& strand,
                build_queue_t& queue,
                nodes_in_progress_t& nodes_in_progess,
                build_queue_node_t* current_node)
      : scheduler_(scheduler),
        strand_(strand),
        queue_(queue),
        nodes_in_progess_(nodes_in_progess),
        current_node_(current_node),
        action_result_(false)
   {
   }

   build_node& node() { return *current_node_->node_; }
   build_queue_node_t& queue_node() { return *current_node_; }

   asio::io_service& scheduler_;
   asio::io_service::strand& strand_;
   build_queue_t& queue_;
   nodes_in_progress_t& nodes_in_progess_;
   build_queue_node_t* current_node_;
   bool action_result_;
};

}

struct builder::impl_t
{
   impl_t(const build_environment& environment, 
          volatile bool& interrupt_flag,
          unsigned worker_count,
          bool unconditional_build) 
      : environment_(environment),
        interrupt_flag_(interrupt_flag),
        worker_count_(worker_count),
        unconditional_build_(unconditional_build)
   {}
   
   result build(nodes_t& nodes, const project* bounds);
   
   build_queue_node_t& gather_nodes(nodes_to_build_t& nodes_to_build, 
                                    build_node& node,
                                    const project* bounds);

   build_queue_node_t& gather_nodes(nodes_to_build_t& nodes_to_build, 
                                    build_queue_node_t& parent_node, 
                                    build_node& node,
                                    const project* bounds);
   void task_completition_handler(shared_ptr<worker_ctx_t> ctx);
   void task_handler(shared_ptr<worker_ctx_t> ctx);

   
   void flatter_queue(unordered_set<const build_queue_node_t*>& result, 
                      const build_queue_node_t& node);
   
   const build_environment& environment_;
   volatile bool& interrupt_flag_;
   unsigned worker_count_;
   bool unconditional_build_;
   boost::object_pool<build_queue_node_t> nodes_pool_;
   result result_;
};

builder::builder(const build_environment& environment, 
                 volatile bool& interrupt_flag,
                 unsigned worker_count,
                 bool unconditional_build) 
   : impl_(new impl_t(environment,
                      interrupt_flag,
                      worker_count,
                      unconditional_build))
{
}

builder::~builder()
{
   delete impl_;
}

builder::result builder::build(build_node& node, const project* bounds)
{
   nodes_t nodes(1, boost::intrusive_ptr<build_node>(&node));
   return impl_->build(nodes, bounds);
}

builder::result builder::build(nodes_t& nodes, const project* bounds)
{
   return impl_->build(nodes, bounds);
}

void builder::impl_t::task_handler(shared_ptr<worker_ctx_t> ctx)
{
   if (ctx->node().action())
   {
      details::buffered_output_environment buffered_environment(environment_);
      ctx->action_result_ = ctx->node().action()->execute(ctx->node(), buffered_environment);
      ctx->node().up_to_date(ctx->action_result_ ? boost::tribool::true_value : boost::tribool::false_value);
   }
   else
   {
      ctx->action_result_ = true;
      ctx->node().up_to_date(boost::tribool::true_value);
   }

   ctx->strand_.post(boost::bind(&impl_t::task_completition_handler, this, ctx));
}

void mark_deps_failed_to_build(build_queue_node_t& node)
{
   node.some_dependencies_failed_to_build_ = true;
   for(build_queue_nodes_t::iterator i = node.uses_nodes_.begin(), last = node.uses_nodes_.end(); i != last; ++i)
      if (!(**i).some_dependencies_failed_to_build_)
         mark_deps_failed_to_build(**i);
}

void builder::impl_t::task_completition_handler(shared_ptr<worker_ctx_t> ctx)
{
   if (interrupt_flag_)
      return;

   if (ctx->current_node_ != NULL)
   {
      ctx->nodes_in_progess_.erase(ctx->current_node_);
         
      if (ctx->action_result_ == true)
      {
         ++result_.updated_targets_;
         for(build_queue_nodes_t::iterator i = ctx->queue_node().uses_nodes_.begin(), last = ctx->queue_node().uses_nodes_.end(); i != last; ++i)
         {
            build_queue_t::nth_index<1>::type::iterator to_modify = ctx->queue_.get<1>().find(*i);
            if (to_modify != ctx->queue_.get<1>().end())
               ctx->queue_.get<1>().modify(ctx->queue_.get<1>().find(*i), dependency_decrementor());
         }
      }
      else
      {
         ++result_.failed_to_build_targets_;
         mark_deps_failed_to_build(ctx->queue_node());
      }
   }

   for(;!ctx->queue_.empty() && ctx->nodes_in_progess_.size() < worker_count_;)
   {
      build_queue_t::nth_index<0>::type::iterator current_node_iterator = ctx->queue_.get<0>().begin();
      build_queue_node_t& current_node = **current_node_iterator;
      if (current_node.dependencies_count_ == 0)
      {
         shared_ptr<worker_ctx_t> worker_ctx(new worker_ctx_t(*ctx));
         worker_ctx->current_node_ = *current_node_iterator;
         worker_ctx->action_result_ = false;
         ctx->nodes_in_progess_.insert(&current_node);
         ctx->scheduler_.post(boost::bind(&impl_t::task_handler, this, worker_ctx));
      }
      else
      {
         if (!current_node.some_dependencies_failed_to_build_)
            return;

         ++result_.skipped_targets_;

         // gather sources that was not built
         nodes_t lack_of_nodes;
         for(build_node::sources_t::const_iterator i = current_node.node_->sources_.begin(), last = current_node.node_->sources_.end(); i != last; ++i)
            if (!i->source_node_->up_to_date())
               lack_of_nodes.push_back(i->source_node_);

         for(nodes_t::const_iterator i = current_node.node_->dependencies_.begin(), last = current_node.node_->dependencies_.end(); i != last; ++i)
            if (!(**i).up_to_date())
               lack_of_nodes.push_back(*i);

         details::buffered_output_environment buffered_environment(environment_);

         // some sources was not build - skip this node, but print why we skip it
         buffered_environment.output_stream() << "...skipped " << current_node.node_->action()->target_tag(*current_node.node_, buffered_environment) << '\n';

         // remove dups
         sort(lack_of_nodes.begin(), lack_of_nodes.end());
         lack_of_nodes.erase(unique(lack_of_nodes.begin(), lack_of_nodes.end()), lack_of_nodes.end());

         for(nodes_t::const_iterator i = lack_of_nodes.begin(), last = lack_of_nodes.end(); i != last; ++i)
            buffered_environment.output_stream()
               << "......for lack of " 
               << ((**i).action() != NULL ? (**i).action()->target_tag(**i, buffered_environment) 
                                          : "?unknown?") 
               << '\n';
      }

      ctx->queue_.get<0>().erase(current_node_iterator);
   }
}

static
bool in_dependencies(const build_node& where,
                     const build_node& what)
{
   for (auto& d : where.dependencies_)
      if (d.get() == &what)
         return true;

   return false;
}

void builder::generate_graphviz(std::ostream& os, const nodes_t& nodes, const project* bounds)
{
   nodes_to_build_t nodes_to_build;
   for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      if (!(**i).up_to_date() || impl_->unconditional_build_)
         impl_->gather_nodes(nodes_to_build, **i, bounds);

   unordered_set<const build_queue_node_t*> all_build_nodes;
   for(nodes_to_build_t::const_iterator i = nodes_to_build.begin(), last = nodes_to_build.end(); i != last; ++i)
      impl_->flatter_queue(all_build_nodes, *i->second);

   os << "digraph g{graph [rankdir = \"LR\"];\n";

   // write nodes
   boost::format node_format("\"%s\" [label = \"%s\" "
                               "shape = \"record\"];\n");
   for(const build_queue_node_t* n : all_build_nodes)
   {
      string labels = (boost::format("%s|dependencies_count = %s") % n % n->dependencies_count_).str();
      labels += "|{action |{ " + (n->node_->action() ? n->node_->action()->name() : string("null")) + "}}";
      // write build node sources 
      {
         labels += "|{src|{";
         bool first = true;
         for(const build_node::source_t& s : n->node_->sources_)
         {
            if (!first)
               labels += "|";
            else
               first = false;
            
            labels += s.source_target_->name();
         }

         labels += "}}";
      }

      // write build node products
      {
         labels += "|{prod|{";
         bool first = true;
         for(const basic_build_target* p : n->node_->products_)
         {
            if (!first)
               labels += "|";
            else
               first = false;

            labels += p->name();
         }

         labels += "}}";
      }

      if (!n->uses_nodes_.empty())
      {
         stringstream s;
         bool first = true;
         for(const build_queue_node_t* un : n->uses_nodes_)
         {
            if (!first)
               s << "|";
            else
               first = false;

            s << '<' << un << '>' << ' ' << un; 
         }

         labels += '|' + s.str();
      }

      os << (node_format % n % labels);
   }

   // write uses edges
   boost::format edge_format("\"%s\" -> \"%s\":\"%s\" [color=%s]\n");
   for(const build_queue_node_t* n : all_build_nodes) {
      for(const build_queue_node_t* un : n->uses_nodes_) {
         const char* color = in_dependencies(*un->node_, *n->node_) ? "blue" : "black";
         os << (edge_format % un % n % un % color);
      }
   }

   os << "}";
}

void builder::impl_t::flatter_queue(unordered_set<const build_queue_node_t*>& result, 
                                    const build_queue_node_t& node)
{
   if (result.find(&node) != result.end())
      return;

   result.insert(&node);

   for(const build_queue_node_t* n : node.uses_nodes_)
      flatter_queue(result, *n);
}

builder::result builder::impl_t::build(nodes_t& nodes, const project* bounds)
{
   result_ = result();

   // gather information about all nodes needed to be built
   nodes_to_build_t nodes_to_build;
   for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      if (!(**i).up_to_date() || unconditional_build_)
         gather_nodes(nodes_to_build, **i, bounds);

   // make build queue
   build_queue_t build_queue;
   for(nodes_to_build_t::const_iterator i = nodes_to_build.begin(), last = nodes_to_build.end(); i != last; ++i)
      build_queue.insert(i->second);
   
   if (build_queue.empty())
      return result_;

   boost::asio::io_service scheduler;
   asio::io_service::strand strand(scheduler);
   nodes_in_progress_t nodes_in_progress;
   
   shared_ptr<worker_ctx_t> initial_ctx(
      new worker_ctx_t(scheduler, strand, build_queue, nodes_in_progress, NULL));
   
   scheduler.post(boost::bind(&impl_t::task_completition_handler, this, initial_ctx));

   boost::thread_group thread_pool;
   if (worker_count_ > 1)
      for(unsigned i = 0; i < worker_count_ - 1; ++i)
         thread_pool.create_thread(boost::bind(&asio::io_service::run, &scheduler));

   scheduler.run();
   
   if (interrupt_flag_)
      throw std::runtime_error("Interrupted by user");

   return result_;
}

build_queue_node_t& 
builder::impl_t::gather_nodes(nodes_to_build_t& nodes_to_build, 
                              build_queue_node_t& parent_node, 
                              build_node& node,
                              const project* bounds)
{
   nodes_to_build_t::const_iterator a = nodes_to_build.find(&node);
   if (a == nodes_to_build.end())
   {
      ++parent_node.dependencies_count_;
      build_queue_node_t& result = gather_nodes(nodes_to_build, node, bounds);
      result.uses_nodes_.insert(&parent_node);

      return result;
   }
   else
   {
      if (a->second->uses_nodes_.insert(&parent_node).second)
         ++parent_node.dependencies_count_;

      return *a->second;
   }
}

build_queue_node_t& builder::impl_t::gather_nodes(nodes_to_build_t& nodes_to_build,
                                                  build_node& node,
                                                  const project* bounds)
{
   build_queue_node_t& ctx_node = *nodes_pool_.construct(&node);
   nodes_to_build.insert(std::make_pair(&node, &ctx_node));

   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      if (!i->source_node_->up_to_date() || unconditional_build_)
         if (bounds == NULL || i->source_node_->products_owner().get_project() == bounds) // we don't build nodes that don't belongs to bounds
            gather_nodes(nodes_to_build, ctx_node, *i->source_node_, bounds);

   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      if (!(**i).up_to_date() || unconditional_build_)
         if (bounds == NULL || (**i).products_owner().get_project() == bounds) // we don't build nodes that don't belongs to bounds
            gather_nodes(nodes_to_build, ctx_node, **i, bounds);

   return ctx_node;
}

}
