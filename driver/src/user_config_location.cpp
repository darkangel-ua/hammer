#include "stdafx.h"
#include "user_config_location.h"
#include <stdlib.h>

hammer::location_t get_user_config_location()
{
#if defined(_WIN32)
   const char* home_path = getenv("USERPROFILE");
   if (home_path != NULL)
      return hammer::location_t(home_path) / "user-config.ham";
   else   
      throw std::runtime_error("Can't find user home directory.");

#else
#   if defined(__linux__)
   
   const char* home_path = getenv("HOME");
   if (home_path != NULL)
      return hammer::location_t(home_path) / "user-config.ham";
   else
      throw std::runtime_error("Can't find user home directory.");
   
#   else
#      error "Platform not supported"
#   endif
#endif
}

