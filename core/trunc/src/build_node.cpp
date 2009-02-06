#include "stdafx.h"
#include "build_node.h"
#include "basic_target.h"

#include <cassert>
#include <algorithm>

namespace hammer{

const basic_target* build_node::find_product(const basic_target* t) const
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

}