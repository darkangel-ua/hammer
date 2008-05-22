#include "stdafx.h"
#include "basic_meta_target.h"
#include "feature_set.h"
#include "feature.h"

namespace hammer{

static void adjust_feature_set(feature_set* f, const basic_meta_target* t)
{
   typedef feature_set::const_iterator iter;
   for(iter i = f->begin(), last = f->end(); i != last; ++i)
   {
      if ((**i).attributes().path)
         (**i).get<feature::path_data>().target_ = t;
   }
}

basic_meta_target::basic_meta_target(const pstring& name, 
                                     feature_set* req, 
                                     feature_set* usage_req)
                                     : name_(name),
                                     requirements_(req),
                                     usage_requirements_(usage_req)
{
   adjust_feature_set(requirements_, this);   
   adjust_feature_set(usage_requirements_, this);   
}

}