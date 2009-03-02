#include "stdafx.h"
#include <hammer/core/pch_main_target.h>
#include <cassert>
#include <hammer/core/types.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type.h>
#include <hammer/core/output_location_strategy.h>

namespace hammer
{

pch_main_target::pch_main_target(const hammer::meta_target* mt, 
                                 const main_target& owner,
                                 const pstring& name, 
                                 const hammer::type* t, 
                                 const feature_set* props,
                                 pool& p)
                                : 
                                 main_target(mt, name, t, props, p),
                                 owner_(owner),
                                 pch_header_(NULL),
                                 pch_source_(NULL),
                                 pch_product_(NULL)
{
}
   
std::vector<boost::intrusive_ptr<build_node> > 
pch_main_target::generate()
{
   const hammer::type& cpp_type = meta_target()->project()->engine()->get_type_registry().get(types::CPP);
   const hammer::type& c_type = meta_target()->project()->engine()->get_type_registry().get(types::C);
   const hammer::type& h_type = meta_target()->project()->engine()->get_type_registry().get(types::H);
   const hammer::type& pch_type = meta_target()->project()->engine()->get_type_registry().get(types::PCH);
   
   typedef std::vector<boost::intrusive_ptr<hammer::build_node> > result_t; 

   // PCH generator consume CPP || C + H and produce OBJ + PCH
   result_t result(main_target::generate());

   for(build_node::sources_t::const_iterator i = result.front()->sources_.begin(), last = result.front()->sources_.end(); i != last; ++i)
   {
      if (i->source_target_->type().equal_or_derived_from(c_type) ||
          i->source_target_->type().equal_or_derived_from(cpp_type))
      {
         pch_source_ = i->source_target_;
      }
      else
         if (i->source_target_->type().equal_or_derived_from(h_type))
            pch_header_ = i->source_target_;
   }

   for(build_node::targets_t::const_iterator i = result.front()->products_.begin(), last = result.front()->products_.end(); i != last; ++i)
   {
      if ((**i).type().equal_or_derived_from(pch_type))
      {
         pch_product_ = *i;
         break;
      }
   }

   return result;
}

location_t pch_main_target::intermediate_dir_impl() const
{
   return meta_target()->project()->engine()->output_location_strategy().compute_output_location(owner_);
}

}