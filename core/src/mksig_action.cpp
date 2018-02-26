#include <hammer/core/build_node.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/feature_set.h>
#include "mksig_action.h"
#include "signature_build_target.h"

namespace hammer {

mksig_action::mksig_action() : build_action("mksig")
{

}

std::string 
mksig_action::target_tag(const build_node& node, 
                         const build_environment& environment) const
{
   assert(node.products_.size() == 1 && dynamic_cast<const signature_build_target*>(node.products_.front()));

   location_t t = relative_path(node.products_owner().intermediate_dir(), environment.current_directory());
   t /= node.products_.front()->name();

   return t.string();
}

bool mksig_action::execute_impl(const build_node& node,
                                const build_environment& environment) const
{
   assert(node.products_.size() == 1 && dynamic_cast<const signature_build_target*>(node.products_.front()));
   const signature_build_target& sig_target = *static_cast<const signature_build_target*>(node.products_.front());
   location_t target_file_name = sig_target.location() / sig_target.name();
   target_file_name.normalize();
   
   std::unique_ptr<std::ostream> f =
      environment.create_output_file(target_file_name.string().c_str(),
                                     std::ios_base::trunc | std::ios_base::out);
   if (!*f) {
      environment.output_stream() << "Can't create file '" << target_file_name.string() << "'";
      return false;
   }

   *f << sig_target.content();

   if (!*f) {
      environment.output_stream() << "Can't write to file '" << target_file_name.string() << "'";
      return false;
   }

   return true;
}

}
