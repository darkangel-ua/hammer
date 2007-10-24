#include "stdafx.h"
#include "types.h"

namespace hammer{ namespace types{

   const type CPP("c++ source file", ".cpp");
   const type LIB("library", "");
   const type H("c++ header file", ".h");
   const type SHARED_LIB("shared library", ".dll");
   const type IMPORT_LIB("import library", ".lib");
   const type STATIC_LIB("static library", ".lib");
   const type EXE("executable", ".exe");
   const type UNKNOWN("unknown type", "");
}}