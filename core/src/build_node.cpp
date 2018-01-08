#include "stdafx.h"
#include <hammer/core/build_node.h>
#include <hammer/core/basic_build_target.h>

#include <cassert>
#include <algorithm>

namespace hammer{

const basic_build_target*
build_node::find_product(const basic_build_target* t) const
{
   targets_t::const_iterator i = std::find(products_.begin(), products_.end(), t);
   if (i != products_.end())
      return *i;
   else 
      return 0;
}

const feature_set& build_node::build_request() const 
{ 
   assert(!products_.empty()); 
   return products_.front()->properties(); 
}

void build_node::up_to_date(boost::tribool::value_t v) 
{ 
   up_to_date_ = v; 
}

}
