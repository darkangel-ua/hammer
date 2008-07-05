#include "stdafx.h"
#include "obj_main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "np_helpers.h"
#include "generator_registry.h"

namespace hammer{

obj_main_target::obj_main_target(const hammer::meta_target* mt, 
                                 const pstring& name, 
                                 const feature_set* props,
                                 pool& p)
   : main_target(mt, name, &mt->project()->engine()->get_type_registry().resolve_from_name(types::OBJ), props, p)
{
}

std::vector<boost::intrusive_ptr<hammer::build_node> > 
obj_main_target::generate()
{
   typedef std::vector<boost::intrusive_ptr<hammer::build_node> > nodes_t ;
   nodes_t result;
       
   for(sources_t::const_iterator i = sources().begin(), last = sources().end(); i != last; ++i)
   {
      pstring new_name = cut_suffix(meta_target()->project()->engine()->pstring_pool(), (**i).name(), (**i).type());
      main_target* mt = new(meta_target()->project()->engine()->targets_pool()) 
                              obj_main_target(meta_target(), 
                              new_name, 
                              &properties(),
                              meta_target()->project()->engine()->targets_pool());
      mt->sources(sources_t(1, *i));
      nodes_t nodes = meta_target()->project()->engine()->generators().construct(mt);
      result.insert(result.end(), nodes.begin(), nodes.end());
   }

   return result;
}

}