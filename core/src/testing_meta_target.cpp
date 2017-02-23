#include "stdafx.h"
#include <hammer/core/testing_meta_target.h>
#include <hammer/core/testing_main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>

namespace hammer{

testing_meta_target::testing_meta_target(hammer::project* p, 
                                         const std::string& name,
                                         const requirements_decl& req, 
                                         const requirements_decl& usage_req,
                                         const hammer::target_type& t)
   : typed_meta_target(p, name, req, usage_req, t)
{

}

main_target* 
testing_meta_target::construct_main_target(const main_target* owner, 
                                           const feature_set* properties) const
{
   main_target* mt = new testing_main_target(this, 
                                             name(), 
                                             &type(), 
                                             properties);
   return mt;
}

}
