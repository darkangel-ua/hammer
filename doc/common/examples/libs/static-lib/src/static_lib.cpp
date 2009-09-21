#include <libs/static_lib.h>
#include <libs/unconstrained_lib.h>

std::string from_static_lib()
{
   return "from_static_lib result + " + from_unconstrained_lib();
}

