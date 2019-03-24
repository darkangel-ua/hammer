#include "stdafx.h"
#include "user_config_location.h"
#include <stdlib.h>

hammer::location_t get_user_config_location()
{
   const char* user_provided_user_config_path = getenv("HAMMER_USER_CONFIG");
   if (user_provided_user_config_path)
      return hammer::location_t(user_provided_user_config_path);

#if defined(_WIN32)
   const char* home_path = getenv("USERPROFILE");
#elif defined(__linux__)
      const char* home_path = getenv("HOME");
#else
#     error "Platform not supported"
#endif

   if (home_path)
      return hammer::location_t(home_path) / "user-config.ham";
   else   
      throw std::runtime_error("Can't find user home directory.");
}

