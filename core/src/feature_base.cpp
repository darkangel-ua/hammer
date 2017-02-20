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

}
