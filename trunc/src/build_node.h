#pragma once
#include <vector>
#include <boost/noncopyable.hpp>

namespace hammer
{
   class basic_target; 

   class build_node : public boost::noncopyable
   {
      public:
         build_node() : up_(0) {}
         build_node* up_;
         std::vector<build_node*> down_;
         std::vector<const basic_target*> sources_;
         std::vector<const basic_target*> products_;
         ~build_node();
   };

   inline build_node::~build_node()
   {
      for(std::vector<build_node*>::iterator i = down_.begin(), last = down_.end(); i != last; ++i)
         delete *i;
   }
}