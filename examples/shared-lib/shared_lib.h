#ifndef SHARED_LIB_H
#define SHARED_LIB_H

#include <string>

#if defined(_MSC_VER)
#   ifdef SHARED_LIB_EXPORTS
#      define SL_EXPORT __declspec (dllexport)
#   else
#      define SL_EXPORT __declspec (dllimport)
#   endif
#else
#   define SL_EXPORT
#endif

std::string SL_EXPORT shared_lib_foo();

#endif
