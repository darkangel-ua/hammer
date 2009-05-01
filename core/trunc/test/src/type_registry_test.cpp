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

/*
BOOST_FIXTURE_TEST_CASE(resolve_from_target_name, type_registry_tests)
{
   pstring src(p_, "lib1");
   BOOST_CHECK_EQUAL(tr_.resolve_from_target_name(src), static_cast<const type*>(0));
}
*/
