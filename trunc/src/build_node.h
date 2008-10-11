#pragma once
#include <vector>
#include <boost/intrusive_ptr.hpp>
#include <algorithm>

namespace hammer
{
   class basic_target; 
   class type;

   class build_node
   {
      public:
         struct source_t
         {
            source_t(const basic_target* source_target, 
                     boost::intrusive_ptr<build_node> source_node) 
                   :
                    source_target_(source_target),
                    source_node_(source_node)
            {}

            const basic_target* source_target_;
            boost::intrusive_ptr<build_node> source_node_;
         };
         
         typedef std::vector<source_t> sources_t;
         typedef std::vector<const basic_target*> targets_t;
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
         build_node() : up_(0), targeting_type_(0), ref_counter_(0) {}
         
         const basic_target* find_product(const basic_target* t) const;
         boost::intrusive_ptr<build_node> up_;
         nodes_t down_;
         sources_t sources_;
         targets_t products_;
         const hammer::type* targeting_type_;
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
   
   inline const basic_target* build_node::find_product(const basic_target* t) const
   {
      targets_t::const_iterator i = std::find(products_.begin(), products_.end(), t);
      if (i != products_.end())
         return *i;
      else 
         return 0;
   }
}