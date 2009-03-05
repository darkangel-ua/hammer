#include "stdafx.h"
#include <stdexcept>
#include <hammer/core/searched_lib_main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/file_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/searched_lib_target.h>

namespace hammer{

searched_lib_main_target::searched_lib_main_target(const hammer::meta_target* mt, 
                                                   const pstring& name, 
                                                   const feature_set* props,
                                                   pool& p)
     : main_target(mt, name, &mt->project()->engine()->get_type_registry().get(types::SEARCHED_LIB), props, p)
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
      basic_target* t = new file_target(this, (**i).value(), &e->get_type_registry().hard_resolve_from_target_name((**i).value()), &properties());
      result->products_.push_back(t);
      result->targeting_type_ = &this->type();
   }
   else
   {
      i = properties().find("name");
      engine* e = mtarget()->meta_target()->project()->engine();
      basic_target* t = new searched_lib_target(this, (**i).value(), &e->get_type_registry().get(types::SEARCHED_LIB), &properties());
      result->products_.push_back(t);
      result->targeting_type_ = &this->type();
   }

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}