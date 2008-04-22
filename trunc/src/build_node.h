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
         build_node() : up_(0), ref_counter_(0) {}
         boost::intrusive_ptr<build_node> up_;
         std::vector<boost::intrusive_ptr<build_node> > down_;
         targets_t sources_;
         targets_t products_;
         unsigned long ref_counter_;
   };

   inline void intrusive_ptr_add_ref(build_node* t)
   {
      ++t->ref_counter_;
   }

   inline void intrusive_ptr_release(build_node* t)
   {
      if (--t->ref_counter_ == 0) delete t;
   }
}