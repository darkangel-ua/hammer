#include "stdafx.h"
#include "build_action.h"

using namespace std;

namespace hammer{

void build_action::execute(const build_node& node, const build_environment& environment) const 
{ 
   string tag(target_tag(node, environment));
   std::cout << name() << ' ' << tag << '\n';

   bool execution_result = false;
   try
   {
      execution_result = execute_impl(node, environment); 
   }
   catch(...)
   {
   }

   if (execution_result == false)
      std::cout << "...failed " << name() << ' ' << tag << '\n';
}

build_action::~build_action()
{

}

}