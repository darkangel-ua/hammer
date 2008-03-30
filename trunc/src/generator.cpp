#include "stdafx.h"
#include "generator.h"

namespace hammer{

generator::generator(const std::string& name,
                     const consumable_types& source_types,
                     const prodused_types& target_types) : 
   name_(name), source_types_(source_types_),
   target_types_(target_types)
{

}

std::vector<basic_target*> 
generator::transform(main_target* to_target, 
                     const std::vector<basic_target*>& from_targets) const
{
   return std::vector<basic_target*>();
}

}