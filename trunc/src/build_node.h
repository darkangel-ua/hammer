#pragma once
#include <vector>
#include <boost/intrusive_ptr.hpp>

namespace hammer
{
   class basic_target; 

   class build_node
   {
      public:
         typedef std::vector<const basic_target*> targets_t;
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
         build_node() : up_(0), ref_counter_(0) {}
         boost::intrusive_ptr<build_node> up_;
         nodes_t down_;
         targets_t sources_;
         targets_t products_;
         mutable unsigned long ref_counter_;
   };

   inline void intrusive_ptr_add_ref(const build_node* t)
   {
      ++t->ref_counter_;
   }

   inline void intrusive_ptr_release(const build_node* t)
   {
      if (--t->ref_counter_ == 0) delete t;
   }
}