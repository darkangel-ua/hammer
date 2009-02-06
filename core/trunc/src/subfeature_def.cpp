#include "stdafx.h"
#include "subfeature_def.h"
#include <stdexcept>

namespace hammer{

subfeature_def::subfeature_def(const std::string& name, 
                               const std::vector<std::string>& legal_values,
                               feature_attributes attrs)
                              :
                               feature_def_base(name, legal_values, attrs)
{
   if (attributes().path ||
       attributes().free ||
       attributes().dependency ||
       attributes().generated)
   {
      throw std::runtime_error("Subfeature '" + name + "' have non-allowed attributes.");
   }
}

}