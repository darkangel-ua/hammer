#include <sstream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <hammer/core/builder.h>
#include <hammer/core/build_action.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/main_target.h>
#include "buffered_output_environment.h"
#include "build_queue.h"

using boost::asio::io_service;
using namespace hammer::details;
using std::string;
using std::stringstream;

namespace hammer {
namespace {

typedef boost::unordered_set<build_queue_node_t*> build_queue_nodes_t;
typedef boost::unordered_set<build_queue_node_t*> nodes_in_progress_t;
typedef boost::unordered_map<const build_node*, build_queue_node_t*> nodes_to_build_t;

struct dependency_decrementor {
   void operator()(build_queue_node_t* v) { --v->dependencies_count_; }
};

struct worker_ctx_t {
   worker_ctx_t(io_service& scheduler,
                io_service::strand& strand,
                build_queue& queue,
                nodes_in_progress_t& nodes_in_progess,
                build_queue_node_t* current_node)
      : scheduler_(scheduler),
        strand_(strand),
        queue_(queue),
        nodes_in_progess_(nodes_in_progess),
        current_node_(current_node),
        action_result_(false)
   {}

   build_node& node() { return *current_node_->node_; }
   details::build_queue_node_t& queue_node() { return *current_node_; }

   io_service& scheduler_;
   io_service::strand& strand_;
   build_queue& queue_;
   nodes_in_progress_t& nodes_in_progess_;
   build_queue_node_t* current_node_;
   bool action_result_;
};

}

struct builder::impl_t {
   impl_t(const build_environment& environment, 
          volatile bool& interrupt_flag,
          unsigned worker_count,
          bool unconditional_build) 
      : environment_(environment),
        interrupt_flag_(interrupt_flag),
        worker_count_(worker_count),
        unconditional_build_(unconditional_build)
   {}
   
   result build(build_nodes_t& nodes,
                const project* bounds);
   
   void task_completition_handler(std::shared_ptr<worker_ctx_t> ctx);
   void task_handler(std::shared_ptr<worker_ctx_t> ctx);

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
   : impl_(new impl_t(environment, interrupt_flag, worker_count, unconditional_build))
{}

builder::~builder()
{
   delete impl_;
}

builder::result
builder::build(build_node& node,
               const project* bounds)
{
   build_nodes_t nodes{boost::intrusive_ptr<build_node>(&node)};
   return impl_->build(nodes, bounds);
}

builder::result
builder::build(build_nodes_t& nodes,
               const project* bounds)
{
   return impl_->build(nodes, bounds);
}

void builder::impl_t::task_handler(std::shared_ptr<worker_ctx_t> ctx)
{
   if (ctx->node().action()) {
      details::buffered_output_environment buffered_environment(environment_);
      ctx->action_result_ = ctx->node().action()->execute(ctx->node(), buffered_environment);
      ctx->node().up_to_date(ctx->action_result_ ? boost::tribool::true_value : boost::tribool::false_value);
   } else {
      ctx->action_result_ = true;
      ctx->node().up_to_date(boost::tribool::true_value);
   }

   ctx->strand_.post(boost::bind(&impl_t::task_completition_handler, this, ctx));
}

static
void mark_deps_failed_to_build(build_queue_node_t& node)
{
   node.some_dependencies_failed_to_build_ = true;
   for (auto& un : node.uses_nodes_) {
      if (!un->some_dependencies_failed_to_build_)
         mark_deps_failed_to_build(*un);
   }
}

void builder::impl_t::task_completition_handler(std::shared_ptr<worker_ctx_t> ctx)
{
   if (interrupt_flag_)
      return;

   if (ctx->current_node_ != nullptr) {
      ctx->nodes_in_progess_.erase(ctx->current_node_);
         
      if (ctx->action_result_ == true) {
         ++result_.updated_targets_;
         for (auto& un : ctx->queue_node().uses_nodes_) {
            auto to_modify = ctx->queue_.get<1>().find(un);
            if (to_modify != ctx->queue_.get<1>().end())
               ctx->queue_.get<1>().modify(ctx->queue_.get<1>().find(un), dependency_decrementor());
         }
      } else {
         ++result_.failed_to_build_targets_;
         mark_deps_failed_to_build(ctx->queue_node());
      }
   }

   for (;!ctx->queue_.empty() && ctx->nodes_in_progess_.size() < worker_count_;) {
      auto current_node_iterator = ctx->queue_.get<0>().begin();
      build_queue_node_t& current_node = **current_node_iterator;
      if (current_node.dependencies_count_ == 0) {
         std::shared_ptr<worker_ctx_t> worker_ctx(new worker_ctx_t(*ctx));
         worker_ctx->current_node_ = *current_node_iterator;
         worker_ctx->action_result_ = false;
         ctx->nodes_in_progess_.insert(&current_node);
         ctx->scheduler_.post(boost::bind(&impl_t::task_handler, this, worker_ctx));
      } else {
         if (!current_node.some_dependencies_failed_to_build_)
            return;

         ++result_.skipped_targets_;

         // gather sources that was not built
         build_nodes_t lack_of_nodes;
         for (auto& source : current_node.node_->sources_) {
            if (!source.source_node_->up_to_date())
               lack_of_nodes.push_back(source.source_node_);
         }

         for (build_node_ptr& dep_node : current_node.node_->dependencies_) {
            if (!dep_node->up_to_date())
               lack_of_nodes.push_back(dep_node);
         }

         details::buffered_output_environment buffered_environment(environment_);

         // some sources was not build - skip this node, but print why we skip it
         if (auto* action = current_node.node_->action().get())
            buffered_environment.output_stream() << "...skipped " << action->target_tag(*current_node.node_, buffered_environment) << '\n';

         // remove dups
         sort(lack_of_nodes.begin(), lack_of_nodes.end());
         lack_of_nodes.erase(unique(lack_of_nodes.begin(), lack_of_nodes.end()), lack_of_nodes.end());

         for (build_node_ptr& node : lack_of_nodes) {
            buffered_environment.output_stream()
               << "......for lack of " 
               << (node->action() ? node->action()->target_tag(*node, buffered_environment)
                                  : "?unknown?")
               << '\n';
         }
      }

      ctx->queue_.get<0>().erase(current_node_iterator);
   }
}

static
bool in_dependencies(const build_node& where,
                     const build_node& what)
{
   for (auto& d : where.dependencies_) {
      if (d.get() == &what)
         return true;
   }

   return false;
}

void builder::generate_graphviz(std::ostream& os,
                                const build_nodes_t& nodes,
                                const project* bounds)
{
   // we know that const_cast is safe here
   details::build_queue build_queue{const_cast<build_nodes_t&>(nodes), true, bounds};

   os << "digraph g{graph [rankdir = \"LR\"];\n";

   // write nodes
   boost::format node_format("\"%s\" [label = \"%s\" "
                               "shape = \"record\"];\n");
   for (const build_queue_node_t* n : build_queue.get<0>()) {
      string labels = (boost::format("%s|dependencies_count = %s") % n % n->dependencies_count_).str();
      labels += "|{action |{ " + (n->node_->action() ? n->node_->action()->name() : string("null")) + "}}";
      // write build node sources 
      {
         labels += "|{src|{";
         bool first = true;
         for (const build_node::source_t& s : n->node_->sources_) {
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
         for (const basic_build_target* p : n->node_->products_) {
            if (!first)
               labels += "|";
            else
               first = false;

            labels += p->name();
         }

         labels += "}}";
      }

      if (!n->uses_nodes_.empty()) {
         stringstream s;
         bool first = true;
         for (const build_queue_node_t* un : n->uses_nodes_) {
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
   for (const build_queue_node_t* n : build_queue.get<0>()) {
      for (const build_queue_node_t* un : n->uses_nodes_) {
         const char* color = in_dependencies(*un->node_, *n->node_) ? "blue" : "black";
         os << (edge_format % un % n % un % color);
      }
   }

   os << "}";
}

builder::result
builder::impl_t::build(build_nodes_t& nodes,
                       const project* bounds)
{
   result_ = result();

   details::build_queue build_queue{nodes, unconditional_build_, bounds};
   
   if (build_queue.empty())
      return result_;

   boost::asio::io_service scheduler;
   io_service::strand strand(scheduler);
   nodes_in_progress_t nodes_in_progress;
   
   std::shared_ptr<worker_ctx_t> initial_ctx(
      new worker_ctx_t(scheduler, strand, build_queue, nodes_in_progress, nullptr));
   
   scheduler.post(boost::bind(&impl_t::task_completition_handler, this, initial_ctx));

   boost::thread_group thread_pool;
   if (worker_count_ > 1) {
      for (unsigned i = 0; i < worker_count_ - 1; ++i)
         thread_pool.create_thread(boost::bind(&io_service::run, &scheduler));
   }

   scheduler.run();
   
   if (interrupt_flag_)
      throw std::runtime_error("Interrupted by user");

   return result_;
}

}
