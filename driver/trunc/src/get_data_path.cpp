#include "stdafx.h"
#include "get_data_path.h"
#include <boost/filesystem/operations.hpp>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace fs = boost::filesystem;

#if defined(_WIN32)

fs::path get_data_path()
{
   char full_module_path[MAX_PATH];
   DWORD res = GetModuleFileName(NULL, full_module_path, MAX_PATH);
   if (res == 0)
      throw std::runtime_error("Can't determinate hammer's data path");
   
   fs::path result(full_module_path);
   return result.parent_path().parent_path();
}

#else
#  if defined(__linux__)

fs::path get_data_path()
{
   fs::path local_path("/usr/local/lib/hammer");
   if (fs::exists(local_path))
      return local_path;
   else
      return "/usr/lib/hammer";
}

#  else
#     error "Unsupported platform"
#  endif
#endif
