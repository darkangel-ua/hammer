#include "stdafx.h"
#include "types.h"
#include <boost/assign/list_of.hpp>
#include <vector>

using namespace std;
using namespace boost::assign;
namespace hammer{ namespace types{

   const type CPP("CPP", ".cpp");
   const type C("C", ".c");
   const type H("H", type::suffixes_t(list_of(".h")(".hpp")));
   const type OBJ("OBJ", ".obj");
   const type SHARED_LIB("SHARED_LIB", ".dll");
   const type STATIC_LIB("STATIC_LIB", ".lib");
   const type IMPORT_LIB("IMPORT_LIB", ".lib");
   const type SEARCHED_LIB("SEARCHED_LIB", ".lib");
   const type HEADER_LIB("HEADER_LIB", type::suffixes_t());
   const type EXE("EXE", ".exe");
   const type UNKNOWN("UNKNOWN_TYPE", "");
}}