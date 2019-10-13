#include "stdafx.h"
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>

namespace hammer{

product_argument_writer::product_argument_writer(const std::string& name,
                                                 const target_type& t,
                                                 output_strategy::value strategy)
   : targets_argument_writer(name, t),
     output_strategy_(strategy)
{
}

argument_writer*
product_argument_writer::clone() const
{
   return new product_argument_writer(*this);
}

bool product_argument_writer::accept(const basic_build_target& t) const
{
   return t.type().equal_or_derived_from(this->source_type());
}

void product_argument_writer::write_impl(std::ostream& output,
                                         const build_node& node,
                                         const build_environment& environment) const
{
   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
   {
      if (accept(**i))
      {
         switch(output_strategy_)
         {
            case output_strategy::RELATIVE_TO_MAIN_TARGET:
            {
               location_t product_path = relative_path((**i).location(), (**i).get_main_target()->location()) / (**i).name();
               output << product_path.string<std::string>();
               break;
            }

            case output_strategy::RELATIVE_TO_WORKING_DIR:
            {
               const location_t full_product_path = (**i).location() / (**i).name();
               const location_t product_path = relative_path(full_product_path, environment.working_directory(**i));

               output << product_path.string();
               break;
            }

            case output_strategy::FULL_PATH:
            {
               const location_t full_product_path = (**i).location() / (**i).name();
               output << full_product_path.string();

               break;
            }

            case output_strategy::FULL_UNC_PATH:
            {
               location_t product_path = (**i).location() / (**i).name();
               product_path.normalize();
               output << "\\\\?\\" << product_path.string<std::string>();
               break;
            }

            default:
               throw std::runtime_error("Unknown output strategy for product");
         }

         break;
      }
   }
}

}
