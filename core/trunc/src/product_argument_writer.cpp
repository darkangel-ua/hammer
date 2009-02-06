#include "stdafx.h"
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/type.h>
#include <hammer/core/build_environment.h>
#include "fs_helpers.h"
#include <hammer/core/main_target.h>

namespace hammer{

product_argument_writer::product_argument_writer(const std::string& name,
                                                 const type& t)
   : targets_argument_writer(name, t)
{
}

argument_writer* product_argument_writer::clone() const
{
   return new product_argument_writer(*this);
}

void product_argument_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
   {
      if ((**i).type().equal_or_derived_from(this->source_type()))
      {
         // FIXME: по идее именно сдесь лучше всего вычислять суфикс для продукта, а не на стадии генераторов
         location_t product_path = relative_path((**i).mtarget()->intermediate_dir(), environment.current_directory()) / (**i).name().to_string();
         output << product_path.native_file_string();
         break;
      }
   }
}

}
