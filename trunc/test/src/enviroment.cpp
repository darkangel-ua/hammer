#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/toolsets/msvc_toolset.h>
#include <hammer/src/copy_generator.h>
#include <hammer/src/generator_registry.h>
#include <hammer/src/testing_generators.h>
#include <hammer/src/toolset_manager.h>

setuped_engine::setuped_engine(bool install_toolsets)
{
   engine_.load_hammer_script(test_data_path / "../../scripts/startup.ham");
   if (install_toolsets)
   {
      engine_.toolset_manager().add_toolset(std::auto_ptr<hammer::toolset>(new hammer::msvc_toolset()));
      engine_.toolset_manager().init_toolset(engine_, "msvc", "8.0", &hammer::location_t());
   }

   hammer::add_testing_generators(engine_, engine_.generators());

   std::auto_ptr<hammer::generator> g(new hammer::copy_generator(engine_));
   engine_.generators().insert(g);
}