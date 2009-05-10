#include "stdafx.h"
#include "get_data_path.h"
#include <Windows.h>

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
   return "/usr/hammer/lib";
}

#  else
#     error "Unsupported platform"
#  endif
#endif
