#include "stdafx.h"
#include "type.h"

namespace hammer{

   type::type(const std::string& name, const std::string& suffix, type* base)
      : name_(name), suffix_(suffix), base_(base)
   {

   }
}