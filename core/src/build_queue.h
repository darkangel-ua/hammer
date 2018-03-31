#pragma once
#include <boost/unordered_set.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/pool/object_pool.hpp>
#include <hammer/core/build_node.h>

namespace hammer {
   class project;
}

namespace hammer { namespace details {

struct build_queue_node_t;
typedef boost::unordered_set<build_queue_node_t*> build_queue_nodes_t;

struct build_queue_node_t
{
   build_queue_node_t(build_node* node)
      : node_(node)
   {}

   build_node* node_;
   unsigned int dependencies_count_ = 0;
   bool some_dependencies_failed_to_build_ = false;
   build_queue_nodes_t uses_nodes_;
};

struct build_queue_dependency_key_extractor
{
   typedef unsigned int result_type;
   result_type operator()(const build_queue_node_t* v) const { return v->dependencies_count_; }
};

using build_queue_internals =
   boost::multi_index::multi_index_container<build_queue_node_t*,
                                             boost::multi_index::indexed_by<boost::multi_index::ordered_non_unique<build_queue_dependency_key_extractor>,
                                                                            boost::multi_index::hashed_unique<boost::multi_index::identity<build_queue_node_t*>>
                                                                           >
                                            >;

class build_queue : private build_queue_internals
{
   public:
      using build_queue_internals::get;
      using build_queue_internals::empty;

      build_queue(build_nodes_t& nodes,
                  const bool unconditional_build,
                  const hammer::project* bounds);

      build_queue(const build_queue&) = delete;
      build_queue& operator = (const build_queue&) = delete;

      ~build_queue();

   private:
      boost::object_pool<build_queue_node_t> pool_;
};


}}
