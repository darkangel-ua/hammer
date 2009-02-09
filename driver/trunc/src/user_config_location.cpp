#include "stdafx.h"
#include "user_config_location.h"
#include <stdlib.h>

hammer::location_t get_user_config_location()
{
#if defined(WIN32) || defined(_WIN32)
   const char* home_path = getenv("USERPROFILE");
   if (home_path != NULL)
      return hammer::location_t(home_path) / "user-config.ham";
   
   return hammer::location_t();

#else
#   error "Platform not supported"
#endif
}

