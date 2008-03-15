#include "stdafx.h"
#include "lib_meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"

namespace hammer{
lib_meta_target::lib_meta_target(hammer::project* p, 
                                 const pstring& name) 
                                : 
                                 meta_target(p, name), type_(0)
{
}

const type* lib_meta_target::instantiate_type() const
{
   if (type_)
      return type_;
   else
   {
      type_ = this->project()->engine()->get_type_registry().resolve_from_name(types::LIB.name());
      return type_;
   }
}

}