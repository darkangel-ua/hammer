#include "stdafx.h"
#include <hammer/core/copy_main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/directory_build_target.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>

namespace hammer{

copy_main_target::copy_main_target(const basic_meta_target* mt,
                                   const std::string& name,
                                   const target_type* t, 
                                   const feature_set* props)
   : main_target(mt, name, t, props),
     recursive_(false)
{
   feature_set::const_iterator d = props->find("destination");
   if (d == props->end())
      throw std::runtime_error("Copy main target must have <destination> feature in requirements");
   
   destination_ = (**d).get_path_data().target_->location() / (**d).value();
   destination_.normalize();

   const type_registry& tr = get_engine()->get_type_registry();
   for(feature_set::const_iterator i = props->find("type-to-copy"), last = props->end(); i != last; i = props->find(i + 1, "type-to-copy"))
      types_to_copy_.push_back(&tr.get(type_tag((**i).value())));

   if (types_to_copy_.empty())
      throw std::runtime_error("Copy main target must have at least one <type-to-copy> feature in requirements");

   // FIXME: By default recursive is on. Because recursive feature is optional, we must turn it on by hands
   // Solution will be specify on what target types feature a works
   // feature.feature recursive : on off : : COPIED ;
   feature_set::const_iterator i = props->find("recursive");
   if ((i != props->end() && (**i).value() == "on") || i == props->end())
   {
      recursive_ = true;
   }
}

void copy_main_target::add_additional_dependencies(hammer::build_node& generated_node) const
{
   directory_build_target* t = new directory_build_target(this, destination());
   boost::intrusive_ptr<hammer::build_node> int_dir_node(new hammer::build_node(*this, false, t->action()));
   int_dir_node->products_.push_back(t);

   generated_node.dependencies_.push_back(int_dir_node);
}

}
