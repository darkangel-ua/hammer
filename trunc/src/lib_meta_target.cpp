#include "stdafx.h"
#include "lib_meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "feature.h"
#include "feature_set.h"

namespace hammer{

lib_meta_target::lib_meta_target(hammer::project* p, 
                                 const pstring& name,
                                 const feature_set* fs) 
                                : 
                                 meta_target(p, name, fs), type_(0)
{
}

const type* lib_meta_target::instantiate_type() const
{
   if (type_)
      return type_;
   else
   {
      const feature* link = properties().find("link");
      if (link)
      {
         if (link->value() == "static")
            type_ = &this->project()->engine()->get_type_registry().resolve_from_name(types::STATIC_LIB.name());
         else
            type_ = &this->project()->engine()->get_type_registry().resolve_from_name(types::SHARED_LIB.name());
      }

      return type_;
   }
}

}