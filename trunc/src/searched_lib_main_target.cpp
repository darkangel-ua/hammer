#include "stdafx.h"
#include <stdexcept>
#include "searched_lib_main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "file_target.h"
#include "feature_set.h"
#include "feature.h"

namespace hammer{

searched_lib_main_target::searched_lib_main_target(const hammer::meta_target* mt, 
                                                   const pstring& name, 
                                                   const feature_set* props,
                                                   pool& p)
     : main_target(mt, name, &mt->project()->engine()->get_type_registry().resolve_from_name(types::SEARCHED_LIB), props, p)
{

}

std::vector<boost::intrusive_ptr<hammer::build_node> > 
searched_lib_main_target::generate()
{
   boost::intrusive_ptr<hammer::build_node> result(new hammer::build_node);
   feature_set::const_iterator i = properties().find("file");
   if (i != properties().end())
   {
      engine* e = mtarget()->meta_target()->project()->engine();
      basic_target* t = new(e->pstring_pool()) file_target(this, (**i).value(), e->get_type_registry().resolve_from_target_name((**i).value()), &properties());
      result->products_.push_back(t);
      result->targeting_type_ = &t->type();
   }
   else
   {
      result->products_.push_back(this);
      result->targeting_type_ = &this->type();
   }

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}