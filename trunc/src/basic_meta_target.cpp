#include "stdafx.h"
#include "basic_meta_target.h"
#include "feature_set.h"
#include "feature.h"
#include "requirements_decl.h"

namespace hammer{

basic_meta_target::basic_meta_target(const pstring& name, 
                                     const requirements_decl& req, 
                                     const requirements_decl& usage_req)
                                     : name_(name),
                                     requirements_(req),
                                     usage_requirements_(usage_req)
{
   requirements_.setup_path_data(this);
   usage_requirements_.setup_path_data(this);
}

basic_meta_target::~basic_meta_target()
{
}

}