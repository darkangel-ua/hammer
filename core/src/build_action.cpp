#include <hammer/core/build_node.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/build_action.h>

using namespace std;

namespace hammer {

bool build_action::execute(const build_node& node,
                           const build_environment& environment,
                           const bool expected_to_fail) const
{ 
   string tag(target_tag(node, environment));
   environment.output_stream() << name() << ' ' << tag << std::endl;

   // FIXME: here we have a situation.. we can't distinguish between 'preparation' failure and
   // actual action execution failure... Right now assume 'preparation` cannot fail
   bool execution_result = false;
   try {
      execution_result = execute_impl(node, environment); 
   } catch(const std::exception& e) {
      environment.output_stream() << "error: " << e.what() << std::endl;
   }

   if (execution_result == false && !expected_to_fail) {
      environment.output_stream() << "...failed " << name() << ' ' << tag << std::endl;
      environment.output_stream() << "...cleaning " << tag << std::endl;
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
      location_t target_path = (**i).location() / (**i).name();
      environment.remove(target_path);
   }
}

}
