#include "stdafx.h"
#include "pch_main_target.h"
#include <cassert>
#include "types.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"

namespace hammer
{

pch_main_target::pch_main_target(const hammer::meta_target* mt, 
                                 const pstring& name, 
                                 const hammer::type* t, 
                                 const feature_set* props,
                                 pool& p)
                                : 
                                 main_target(mt, name, t, props, p),
                                 pch_header_(NULL),
                                 pch_source_(NULL)
{
   feature_set* modified_properties = props->clone();
   feature* create_pch_feature = meta_target()->project()->engine()->feature_registry().create_feature("__create_pch", NULL);
   create_pch_feature->get_generated_data().target_ = this;
   modified_properties->join(create_pch_feature);
   modified_properties->join("__pch", NULL);
   properties(modified_properties);
}
   
std::vector<boost::intrusive_ptr<build_node> > 
pch_main_target::generate()
{
   const hammer::type& cpp_type = meta_target()->project()->engine()->get_type_registry().resolve_from_name(types::CPP);
   const hammer::type& c_type = meta_target()->project()->engine()->get_type_registry().resolve_from_name(types::C);
   const hammer::type& h_type = meta_target()->project()->engine()->get_type_registry().resolve_from_name(types::H);
   
   typedef std::vector<boost::intrusive_ptr<hammer::build_node> > result_t; 

   // PCH generator consume CPP || C + H and produce OBJ + PCH
   result_t result(main_target::generate());

   for(build_node::sources_t::const_iterator i = result.front()->sources_.begin(), last = result.front()->sources_.end(); i != last; ++i)
   {
      if (c_type == i->source_target_->type() ||
          cpp_type == i->source_target_->type())
      {
         pch_source_ = i->source_target_;
      }
      else
         if (h_type == i->source_target_->type())
            pch_header_ = i->source_target_;
   }

   return result;
}

}