#if !defined(h_e6feb123_26ee_4388_a943_1192b459e361)
#define h_e6feb123_26ee_4388_a943_1192b459e361

#if defined(LIB1_STATIC_LIB)
#  define LIB1_API
#else
#  if defined(LIB1_EXPORTS)
#     if defined(_MSC_VER) || defined(__MINGW32__)
#        define LIB1_API __declspec(dllexport)
#     endif
#     if defined(__BORLANDC__)
#        define LIB1_API _export      
#     endif
#     if defined(__linux__)
#        define LIB1_API
#     endif
#  else
#     if defined(_MSC_VER) || defined(__MINGW32__)
#        define LIB1_API __declspec(dllimport)
#     endif
#     if defined(__BORLANDC__)
#        define LIB1_API _export      
#     endif
#     if defined(__linux__)
#        define LIB1_API
#     endif
#  endif
#endif


#endif //h_e6feb123_26ee_4388_a943_1192b459e361
