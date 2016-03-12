#include "stdafx.h"
#include <hammer/core/pch_argument_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/pch_main_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>

namespace hammer{

void pch_argument_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   const feature_set& build_request = node.build_request();
   feature_set::const_iterator pch_iter = build_request.find("pch");
   if (pch_iter == build_request.end() || (**pch_iter).value() == "off")
      return;
 
   switch(part_)
   {
      case part::header:
      {
         const pch_main_target* pch_target = static_cast<const pch_main_target*>((**pch_iter).get_generated_data().target_);
         location_t pch_header(pch_target->pch_header().name().to_string());
         output << prefix_ << pch_header.leaf() << suffix_;
         break;
      }

      case part::product:
      {
         const pch_main_target* pch_target = static_cast<const pch_main_target*>((**pch_iter).get_generated_data().target_);
         location_t pch_product_location(pch_target->pch_product().location() / pch_target->pch_product().name().to_string());
//         location_t output_path = relative_path(pch_product_location, environment.current_directory());
         location_t output_path = relative_path(pch_product_location, pch_target->location());
         output_path.normalize();

         output << prefix_ << output_path.string() << suffix_;
         break;
      }
   }
}

}

