#include "stdafx.h"
#include <hammer/core/subfeature.h>
#include <hammer/core/subfeature_def.h>

namespace hammer{

subfeature::subfeature(const subfeature_def& def,
                       const std::string& value)
   : definition_(&def),
     value_(value)
{

}

subfeature::~subfeature()
{

}

const std::string& subfeature::name() const
{
   return definition_->name();
}

bool subfeature::operator == (const subfeature& rhs) const
{
   return definition_->name() == rhs.definition().name() && value_ == rhs.value();
}

}
