#include "stdafx.h"
#include "copy_main_target.h"
#include "feature_set.h"
#include "feature.h"
#include "basic_meta_target.h"
#include "directory_target.h"
#include "type_registry.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"

namespace hammer{

copy_main_target::copy_main_target(const hammer::meta_target* mt, 
                                   const pstring& name, 
                                   const hammer::type* t, 
                                   const feature_set* props,
                                   pool& p)
   : main_target(mt, name, t, props, p),
     recursive_(false)
{
   feature_set::const_iterator d = props->find("destination");
   if (d == props->end())
      throw std::runtime_error("Copy main target must have <destination> feature in requirements");
   
   destination_ = (**d).get_path_data().target_->location() / (**d).value().to_string();
   destination_.normalize();

   const type_registry& tr = mt->project()->engine()->get_type_registry();
   for(feature_set::const_iterator i = props->find("type-to-copy"), last = props->end(); i != last; i = props->find(i + 1, "type-to-copy"))
      types_to_copy_.push_back(&tr.get(type_tag((**i).value().to_string())));

   if (types_to_copy_.empty())
      throw std::runtime_error("Copy main target must have at least one <type-to-copy> feature in requirements");

   // FIXME: By default recursive is on. Because recursive feature is optional, we must turn it on by hands
   // Solution will be specify on what target types feature a works
   // feature.feature recursive : on off : : COPIED ;
   feature_set::const_iterator i = props->find("recursive");
   if (i != props->end() && (**i).value() == "on" ||
       i == props->end())
   {
      recursive_ = true;
   }
}

void copy_main_target::add_additional_dependencies(hammer::build_node& generated_node) const
{
   boost::intrusive_ptr<hammer::build_node> int_dir_node(new hammer::build_node);
   int_dir_node->products_.push_back(new directory_target(this, destination()));
   int_dir_node->action(static_cast<const directory_target*>(int_dir_node->products_.front())->action());

   generated_node.dependencies_.push_back(int_dir_node);
}

}
