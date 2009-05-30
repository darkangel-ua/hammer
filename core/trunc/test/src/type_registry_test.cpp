#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/pool.h>

using namespace hammer;
using namespace std;

struct type_registry_tests
{
   type_registry_tests() : fr_(&p_)
   {
      tr_.insert(target_type(types::STATIC_LIB, ".lib"));
   }

   type_registry tr_;
   pool p_;
   feature_registry fr_;

};

