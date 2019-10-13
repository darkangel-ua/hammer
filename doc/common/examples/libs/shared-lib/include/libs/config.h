#if !defined(h_5d1cda2a_e317_4d9a_8078_d55ca123ca7a)
#define h_5d1cda2a_e317_4d9a_8078_d55ca123ca7a

#if defined(SHARED_LIB_EXPORTS)
#   if defined(_MSC_VER)
#      define SHARED_LIB_API __declspec(dllexport)
#   endif
#   if defined(__BORLANDC__)
#      define SHARED_LIB_API _export      
#   endif
#   if defined(__linux__)
#      define SHARED_LIB_API
#   endif
#else
#   if defined(_MSC_VER)
#      define SHARED_LIB_API __declspec(dllimport)
#   endif
#   if defined(__BORLANDC__)
#      define SHARED_LIB_API _export      
#   endif
#   if defined(__linux__)
#      define SHARED_LIB_API
#   endif
#endif

#endif //h_5d1cda2a_e317_4d9a_8078_d55ca123ca7a
