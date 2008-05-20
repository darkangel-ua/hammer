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
                                 const feature_set* fs,
                                 const feature_set* usage_req) 
                                : 
                                 meta_target(p, name, fs, usage_req)
{
}

const type* lib_meta_target::instantiate_type(const feature_set& fs) const
{
   feature_set::const_iterator link = fs.find("link");
   if (link != fs.end())
   {
      if ((*link)->value() == "static")
         return &this->project()->engine()->get_type_registry().resolve_from_name(types::STATIC_LIB);
      else
         return &this->project()->engine()->get_type_registry().resolve_from_name(types::SHARED_LIB);
   }

   assert(false && "This is must be unreachible.");
   return 0;
}

}