#if !defined(h_8df18f70_e30a_4eb4_bfb8_e89fb04f1e56)
#define h_8df18f70_e30a_4eb4_bfb8_e89fb04f1e56

#if defined(UNCONSTRAINED_LIB_STATIC_LIB)
#  define UNCONSTRAINED_LIB_API
#else
#  if defined(UNCONSTRAINED_LIB_EXPORTS)
#     if defined(_MSC_VER)
#        define UNCONSTRAINED_LIB_API __declspec(dllexport)
#     endif
#     if defined(__BORLANDC__)
#        define UNCONSTRAINED_LIB_API _export      
#     endif
#     if defined(__linux__)
#        define UNCONSTRAINED_LIB_API
#     endif
#  else
#     if defined(_MSC_VER)
#        define UNCONSTRAINED_LIB_API __declspec(dllimport)
#     endif
#     if defined(__BORLANDC__)
#        define UNCONSTRAINED_LIB_API _export      
#     endif
#     if defined(__linux__)
#        define UNCONSTRAINED_LIB_API
#     endif
#  endif
#endif

#endif //h_8df18f70_e30a_4eb4_bfb8_e89fb04f1e56
