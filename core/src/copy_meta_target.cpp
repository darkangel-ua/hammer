#include <hammer/core/copy_meta_target.h>
#include <hammer/core/copy_main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>

namespace hammer {

static
location_t
make_destination(const location_t& original_dest,
                 const location_t& project_location)
{
   if (original_dest.has_root_path())
      return original_dest;

   auto result = project_location / original_dest;
   result.normalize();

   return result;
}

copy_meta_target::copy_meta_target(hammer::project* p,
                                   const std::string& name,
                                   const location_t& destination,
                                   const std::vector<const target_type*>& types_to_copy,
                                   const bool recursive)
   : meta_target(p, name, {}, {}),
     destination_(make_destination(destination, p->location())),
     types_to_copy_(types_to_copy),
     recursive_(recursive)
{
}

main_target*
copy_meta_target::construct_main_target(const main_target* owner,
                                        const feature_set* properties) const
{
   main_target* mt = new copy_main_target(this, 
                                          name(), 
                                          properties);
   return mt;
}

}
