#include "stdafx.h"
#include <cassert>
#include <algorithm>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/build_node.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>

namespace hammer {

const basic_build_target*
build_node::find_product(const basic_build_target* t) const {
   auto i = std::find(products_.begin(), products_.end(), t);
   return i != products_.end() ? *i : nullptr;
}

const basic_build_target*
build_node::find_product(const type_tag& tag) const {
   const auto& type = products_owner_->get_engine().get_type_registry().get(tag);
   auto i = std::find_if(products_.begin(), products_.end(), [&] (const basic_build_target* bt) {
      return bt->type().equal_or_derived_from(type);
   });

   return i != products_.end() ? *i : nullptr;
}

const feature_set&
build_node::build_request() const {
   assert(!products_.empty()); 
   return products_.front()->properties(); 
}

void build_node::up_to_date(boost::tribool::value_t v) { 
   up_to_date_ = v; 
}

}
