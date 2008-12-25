#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/msvc_generator.h>
#include <hammer/src/copy_generator.h>
#include <hammer/src/generator_registry.h>

setuped_engine::setuped_engine()
{
   engine_.load_hammer_script(test_data_path / "../../scripts/startup.ham");
   hammer::add_msvc_generators(engine_, engine_.generators());

   std::auto_ptr<hammer::generator> g(new hammer::copy_generator(engine_));
   engine_.generators().insert(g);
}