#include "stdafx.h"
#include "main_target.h"

namespace hammer{

   main_target::main_target(const pstring& name, const hammer::type* t) 
      : basic_target(t) 

   {
   }

   void main_target::sources(const std::vector<basic_target*>& srcs)
   {
      sources_ = srcs;
   }

}
