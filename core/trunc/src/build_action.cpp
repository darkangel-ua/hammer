#include "stdafx.h"
#include "build_action.h"
#include "build_node.h"
#include "basic_target.h"
#include "build_environment.h"

using namespace std;

namespace hammer{

bool build_action::execute(const build_node& node, const build_environment& environment) const 
{ 
   string tag(target_tag(node, environment));
   std::cout << name() << ' ' << tag << '\n';

   bool execution_result = false;
   try
   {
      execution_result = execute_impl(node, environment); 
   }
   catch(const std::exception& e)
   {
      std::cout << "error: " << e.what() << '\n';
   }

   if (execution_result == false)
   {
      std::cout << "...failed " << name() << ' ' << tag << '\n';
      std::cout << "...cleaning " << tag << '\n';
      clean_on_fail(node, environment);
   }

   return execution_result;
}

build_action::~build_action()
{

}

void build_action::clean_on_fail(const build_node& node, const build_environment& environment) const
{
   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
   {
      location_t target_path = (**i).location() / (**i).name().to_string();
      environment.remove(target_path);
   }
}

}