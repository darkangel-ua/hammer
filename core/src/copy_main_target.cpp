#include <boost/filesystem/operations.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/directory_build_target.h>
#include <hammer/core/types.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/copy_meta_target.h>
#include <hammer/core/copy_main_target.h>
#include <hammer/core/collect_nodes.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/product_argument_writer.h>

namespace hammer {
namespace {

class copy_directory_build_target : public directory_build_target {
   public:
      copy_directory_build_target(const main_target* mt,
                                  const location_t& dir)
         : directory_build_target(mt, dir)
      {}

      bool clean(const build_environment& environment) const override;
};

bool copy_directory_build_target::clean(const build_environment& environment) const
{
   // its ok to not delete directory if its not empty, because we might use destination where other files placed
   if (exists(location()) && !is_empty(location()))
      return directory_build_target::clean(environment);
   else
      return false;
}

class copy_action : public build_action {
   public:
      copy_action()
         : build_action("copy-file")
      {}

      std::string
      target_tag(const build_node& node,
                 const build_environment& environment) const override;
      std::vector<feature_ref>
      valuable_features() const override { return {}; }

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;
      void clean_on_fail(const build_node& node,
                         const build_environment& environment) const override;
};

std::string
copy_action::target_tag(const build_node& node,
                        const build_environment& environment) const
{
   std::ostringstream s;
   product_argument_writer writer({}, *node.targeting_type_, product_argument_writer::output_strategy::RELATIVE_TO_WORKING_DIR);
   writer.write(s, node, environment);

   return s.str();
}

bool copy_action::execute_impl(const build_node& node,
                               const build_environment& environment) const
{
   assert(node.products_.size() == 1);
   assert(node.sources_.size() == 1);

   location_t destination = node.products_.front()->location() /
                            node.products_.front()->name();
   location_t source = node.sources_.front().source_target_->location() /
                       node.sources_.front().source_target_->name();
   destination.normalize();
   source.normalize();

   if (exists(destination))
      environment.remove(destination);

   environment.copy(source, destination);

   return true;
}

void copy_action::clean_on_fail(const build_node& node,
                                const build_environment& environment) const
{
}

}

copy_main_target::copy_main_target(const basic_meta_target* mt,
                                   const std::string& name,
                                   const feature_set* props)
   : main_target(mt, name, &mt->get_engine().get_type_registry().get(types::COPY), props)
{
   assert(dynamic_cast<const copy_meta_target*>(mt));
}

using visited_nodes_t = std::set<const build_node*>;

build_nodes_t
copy_main_target::generate_impl() const
{
   build_nodes_t sources_nodes;
   for (const basic_target* source : sources()) {
      build_nodes_t nodes = source->generate();
      sources_nodes.insert(sources_nodes.end(), nodes.begin(), nodes.end());
   }

   const copy_meta_target& owner = static_cast<const copy_meta_target&>(*get_meta_target());
   build_node::sources_t collected_nodes;
   visited_nodes_t visited_nodes;
   collect_nodes(collected_nodes, visited_nodes, sources_nodes, owner.types_to_copy_, owner.recursive_);

   auto cp_action = std::make_shared<copy_action>();
   build_nodes_t result;
   for (auto source : collected_nodes) {
      build_node_ptr new_node{new hammer::build_node(*this, false, cp_action)};
      new_node->targeting_type_ = &source.source_target_->type();
      new_node->sources_.push_back(source);
      new_node->down_.push_back(source.source_node_);

      basic_build_target* new_target =
         new generated_build_target(this,
                                    source.source_target_->name(),
                                    source.source_target_->hash(),
                                    &source.source_target_->type(),
                                    &source.source_target_->properties(),
                                    &owner.destination_);
      new_node->products_.push_back(new_target);

      result.push_back(new_node);
   }

   auto dir_target = new copy_directory_build_target(this, owner.destination_);
   build_node_ptr dest_dir_node{new hammer::build_node(*this, false, dir_target->action())};
   dest_dir_node->products_.push_back(dir_target);

   for (build_node_ptr& node : result)
      node->dependencies_.push_back(dest_dir_node);

   return result;
}

}
