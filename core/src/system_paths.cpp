#include <boost/filesystem/operations.hpp>
#include <hammer/core/system_paths.h>

namespace fs = boost::filesystem;

namespace hammer {

fs::path
get_data_folder() {
#if defined(_WIN32)
   const char* c_data_path = getenv("APPDATA");
   if (!c_data_path)
      throw std::runtime_error("Can't find user application data folder");

   auto data_path = fs::path(c_data_path) / "hammer";
#else
   // we already checked that HOME exists when did get_user_config_path
   auto home = fs::path(getenv("HOME"));
   auto data_path = [&] {
      if (fs::exists(home / ".local"))
         return home / ".local" / "hammer";

      return home / ".hammer";
   }();
#endif

   if (!fs::exists(data_path))
      fs::create_directories(data_path);

   return data_path;
}

fs::path
get_user_config_path() {
   const char* user_provided_user_config_path = getenv("HAMMER_USER_CONFIG");
   if (user_provided_user_config_path)
      return fs::path(user_provided_user_config_path);

#if defined(_WIN32)
   const char* c_home_path = getenv("USERPROFILE");
#else
   const char* c_home_path = getenv("HOME");
#endif

   if (!c_home_path)
      throw std::runtime_error("Can't find user home folder");

   const auto home_path = fs::path(c_home_path);

#if defined(_WIN32)
   return home_path / "user-config.ham";
#else
   if (fs::exists(home_path / ".config"))
      return home_path / ".config" / "hammer" / "user-config.ham";

   return home_path / "user-config.ham";
#endif
}

system_paths
get_system_paths() {
   return { get_user_config_path(), get_data_folder() };
}

}
