#include <libs/shared_lib.h>
#include <libs/unconstrained_lib.h>

std::string from_shared_lib()
{
   return "from_shared_lib result + " + from_unconstrained_lib();
}

