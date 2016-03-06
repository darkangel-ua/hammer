#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/copy_generator.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/types.h>


setuped_engine::setuped_engine(bool install_toolsets)
{
   engine_.load_hammer_script(test_data_path / "../scripts/startup.ham");
   hammer::types::register_standart_types(engine_.get_type_registry(), engine_.feature_registry());
   if (install_toolsets)
   {
      engine_.toolset_manager().add_toolset(std::auto_ptr<hammer::toolset>(new hammer::msvc_toolset()));
      hammer::location_t empty;
      engine_.toolset_manager().init_toolset(engine_, "msvc", "8.0", &empty);
   }

   hammer::add_testing_generators(engine_, engine_.generators());

   std::auto_ptr<hammer::generator> g(new hammer::copy_generator(engine_));
   engine_.generators().insert(g);
}
