#include <libs/unconstrained_lib.h>

#if defined(UNCONSTRAINED_LIB_EXPORTS)

std::string UNCONSTRAINED_LIB_API from_unconstrained_lib()
{
   return "from_unconstrained_lib as shared lib result";
}

#else

std::string UNCONSTRAINED_LIB_API from_unconstrained_lib()
{
   return "from_unconstrained_lib as static lib result";
}

#endif
