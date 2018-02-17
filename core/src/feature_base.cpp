#include "stdafx.h"
#include <hammer/core/feature_base.h>
#include <cassert>

namespace hammer{

feature_base::feature_base(const feature_def* def,
                           const std::string& value)
   : definition_(def),
     value_(value)
{
   assert(def && "Definition cannot be NULL");   
}

static
feature_value_ns_ptr global_ns;

const feature_value_ns_ptr&
feature_base::get_value_ns() const
{
   if (attributes().no_checks)
      return global_ns;
   else
      return definition_->get_legal_value_ns(value());
}

}
