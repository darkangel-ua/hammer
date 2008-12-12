#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/msvc_generator.h>

setuped_engine::setuped_engine()
{
   engine_.load_hammer_script(test_data_path / "../../scripts/startup.ham");
   hammer::add_msvc_generators(engine_, engine_.generators());
}