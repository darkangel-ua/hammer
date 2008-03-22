#include "stdafx.h"
#include "generator.h"

namespace hammer{

generator::generator(const std::string& name,
                     const types_t& source_types,
                     const types_t& target_types) : 
   name_(name), source_types_(source_types_),
   target_types_(target_types)
{

}

}