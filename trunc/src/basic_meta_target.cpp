#include "stdafx.h"
#include "basic_meta_target.h"
#include "feature_set.h"
#include "feature.h"
#include "requirements_decl.h"

namespace hammer{

basic_meta_target::basic_meta_target(const pstring& name, 
                                     const requirements_decl& req, 
                                     feature_set* usage_req)
                                     : name_(name),
                                     requirements_(req),
                                     usage_requirements_(usage_req)
{
   requirements_.setup_path_data(this);
   set_path_data(usage_requirements_, this);
}

basic_meta_target::~basic_meta_target()
{
}

}