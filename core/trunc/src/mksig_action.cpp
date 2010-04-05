#include "stdafx.h"
#include "mksig_action.h"
#include <hammer/core/build_node.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/feature_set.h>

namespace hammer{

mksig_action::mksig_action() : build_action("mksig")
{
}

std::string 
mksig_action::target_tag(const build_node& node, 
                         const build_environment& environment) const
{
   assert(node.products_.size() == 1);

   location_t t = relative_path(node.products_owner().intermediate_dir(), environment.current_directory());
   t /= node.products_.front()->name().to_string();

   return t.native_file_string();
}

bool mksig_action::execute_impl(const build_node& node, const build_environment& environment) const
{
   location_t target_file_name = node.products_.front()->location() / node.products_.front()->name().to_string();
   target_file_name.normalize();
   
   std::auto_ptr<std::ostream> f = 
      environment.create_output_file(target_file_name.native_file_string().c_str(), 
                                     std::ios_base::trunc | std::ios_base::out);
   if (!*f)
   {
      environment.output_stream() << "Can't create file '" << target_file_name.native_file_string() << "'";
      return false;
   }

   dump_for_hash(*f, node.products_owner().properties(), true);
   if (!*f)
   {
      environment.output_stream() << "Can't write to file '" << target_file_name.native_file_string() << "'";
      return false;
   }

   return true;
}

}
