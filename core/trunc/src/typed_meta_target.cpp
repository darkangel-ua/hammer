#include "stdafx.h"
#include "typed_meta_target.h"
#include "project.h"
#include "engine.h"
#include "main_target.h"

namespace hammer{

main_target* typed_meta_target::construct_main_target(const feature_set* properties) const
{
   main_target* mt = new main_target(this, 
                                     name(), 
                                     type_, 
                                     properties,
                                     project()->engine()->targets_pool());
   return mt;
}

}