#include "stdafx.h"
#include <hammer/core/generator.h>
#include <hammer/core/target_type.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/generated_build_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/build_action.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_set.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/crypto/md5.hpp>

namespace hammer {

static
std::vector<const feature*> empty_valuable_features;

static
std::vector<const feature*>
make_generator_valuable_properties(const bool composite,
                                   feature_registry& fr)
{
   if (composite) {
      std::vector<const feature*> result;
      append_valuable_feature(result, *fr.create_feature("name", ""), fr);
      return result;
   } else
      return empty_valuable_features;
}

generator::generator(hammer::engine& e,
                     const std::string& name,
                     const consumable_types_t& source_types,
                     const producable_types_t& target_types,
                     bool composite,
                     const build_action_ptr& action,
                     const feature_set* c) :
   engine_(&e), name_(name), source_types_(source_types),
   target_types_(target_types),
   composite_(composite),
   constraints_(c),
   action_(action),
   include_composite_generators_(false),
   action_valuable_features_( action ? action->valuable_features() : std::vector<const feature*>()),
   constraints_valuable_features_(c ? make_valuable_features(*c) : std::vector<const feature*>()),
   generator_valuable_features_(make_generator_valuable_properties(composite, e.feature_registry()))
{
}

bool generator::is_consumable(const target_type& t) const
{
   if (consumable_types().empty())
      return true;

   for(generator::consumable_types_t::const_iterator i = consumable_types().begin(), last = consumable_types().end(); i != last; ++i)
      if (t.equal_or_derived_from(*i->type_))
         return true;

   return false;
}

basic_build_target*
generator::create_target(const main_target* mt,
                         const build_node::sources_t& sources,
                         const std::string* composite_target_name,
                         const produced_type& type,
                         const feature_set* f) const
{
   auto nh = make_product_name_and_hash(sources, composite_target_name, type, *f);
   return new generated_build_target(mt, nh.first, nh.second, type.type_, f);
}

// we making generated target dependent on sources hashes to rebuild it when
// sources specific properties changes
static
std::string calculate_hash(const build_node::sources_t& sources,
                           const feature_set& props)
{
   std::set<std::string> sources_hashes;
   for (auto& s : sources)
      if (!s.source_target_->hash().empty())
         sources_hashes.insert(s.source_target_->hash());

   std::ostringstream hash_stream;
   dump_for_hash(hash_stream, props, true);

   for (auto& h : sources_hashes)
      hash_stream << ' ' << h;

   return boost::crypto::md5(hash_stream.str()).to_string();
}

static
void add_version(std::string& s,
                 const feature_set& properties)
{
   feature_set::const_iterator i = properties.find("version");
   if (i != properties.end())
      s += '-' + (**i).value();
}

std::pair<std::string /*name*/, std::string /*hash*/>
generator::make_product_name_and_hash(const build_node::sources_t& sources,
                                      const std::string* composite_target_name,
                                      const produced_type& product_type,
                                      const feature_set& product_properties)
{
   const std::string hash = calculate_hash(sources, product_properties);
   auto p_name = product_properties.find("name");
   const std::string pure_name = [&]() -> std::string {
      if (composite_target_name) {
         if ( p_name != product_properties.end())
            return (**p_name).value();

         return *composite_target_name;
      }

      auto& source_target = *sources.front().source_target_;
      auto& source_suffix = source_target.type().suffix_for(source_target.name(), source_target.properties());

      return std::string(source_target.name().begin(), source_target.name().begin() + (source_target.name().size() - source_suffix.size()));
   }();

   const std::string& product_name = [&]()  -> std::string {
      if (p_name == product_properties.end()) {
         std::string result = product_type.type_->prefix_for(product_properties) + pure_name;
         if (product_type.need_tag_ && p_name == product_properties.end()) {
            add_version(result, product_properties);
            result += '-' + hash;
         }
         result += product_type.type_->suffix_for(product_properties);

         return result;
      } else
         return pure_name;
   }();

   return { product_name, hash };
}

build_nodes_t
generator::construct(const target_type& type_to_construct,
                     const feature_set& props,
                     const build_nodes_t& sources,
                     const basic_build_target* source_target,
                     const std::string* composite_target_name,
                     const main_target& owner) const
{
   const auto& source_valuable_features = source_target ? source_target->valuable_features()
                                                        : sources.size() == 1 && sources.front()->products_.size() == 1
                                                        ? sources.front()->products_.front()->valuable_features() : empty_valuable_features;
   const feature_set* valuable_properties = make_valuable_properties(props,
                                                                     type_to_construct.valuable_features(),
                                                                     source_valuable_features);

   if (source_target) {
      // this is one-to-one construction
      assert(sources.size() == 1);

      build_node_ptr result(new build_node(owner, composite_, action()));
      result->sources_.push_back(build_node::source_t(source_target, sources.front()));
      result->down_.push_back(sources.front());
      result->products_.push_back(create_target(&owner, result->sources_, nullptr, producable_types().front(), valuable_properties));
      result->targeting_type_ = &type_to_construct;
      return build_nodes_t(1, result);
   } else {
      // this is many-to-one(+additional) construction
      build_node_ptr result(new build_node(owner, composite_, action()));

      typedef build_nodes_t::const_iterator iter;
      for(iter i = sources.begin(), last = sources.end(); i != last; ++i) {
         bool node_added = false;
         for(build_node::targets_t::const_iterator p_i = (**i).products_.begin(), p_last = (**i).products_.end(); p_i != p_last; ++p_i) {
            if (is_consumable((**p_i).type())) {
               result->sources_.push_back(build_node::source_t(*p_i, *i));
               if (!node_added) {
                  result->down_.push_back(*i);
                  node_added = true;
               }
            }
         }
      }

      for(auto type : target_types_)
         result->products_.push_back(create_target(&owner, result->sources_, composite_target_name, type, valuable_properties));

      result->targeting_type_ = &type_to_construct;
      return build_nodes_t(1, result);
  }
}

generator::~generator()
{

}

void remove_dups(build_node::nodes_t& nodes)
{
   using namespace boost::multi_index;
   typedef boost::intrusive_ptr<build_node> node_t;
   typedef multi_index_container<node_t, indexed_by<sequenced<>, ordered_unique<identity<node_t> > > > container_t;

   if (nodes.empty())
      return;

   container_t c;
   container_t::nth_index<0>::type& idx = c.get<0>();
   for(build_node::nodes_t::const_reverse_iterator i = nodes.rbegin(), last = nodes.rend(); i != last; ++i)
      idx.push_back(*i);

   build_node::nodes_t result;
   result.reserve(c.get<0>().size());
   typedef container_t::nth_index<0>::type idx_0_t;
   for(idx_0_t::const_reverse_iterator i = c.get<0>().rbegin(), last = c.get<0>().rend(); i != last; ++i)
      result.push_back(*i);
   
   nodes.swap(result);
}

generator::producable_types_t
make_product_types(engine& e,
                   const std::vector<type_tag>& types)
{
   generator::producable_types_t result;
   for(const type_tag& t : types)
      result.push_back(e.get_type_registry().get(t));

   return result;
}

generator::consumable_types_t
make_consume_types(engine& e,
                   const std::vector<type_tag>& types)
{
   generator::consumable_types_t result;
   for(const type_tag& t : types)
      result.push_back(e.get_type_registry().get(t));

   return result;
}

generator::consumable_types_t
make_consume_types(const generator::producable_types_t& types)
{
   generator::consumable_types_t result;
   for(const generator::produced_type& t : types)
      result.push_back(*t.type_);

   return result;
}

feature_set*
generator::make_valuable_properties(const feature_set& target_props,
                                    const std::vector<const feature*>& target_type_valuable_features,
                                    const std::vector<const feature*>& source_target_valuable_features) const
{
   feature_set* result = target_props.owner().make_set();

   auto all_valuable_features = action_valuable_features_;
   merge(all_valuable_features, constraints_valuable_features_);
   merge(all_valuable_features, generator_valuable_features_);
   merge(all_valuable_features, target_type_valuable_features);
   merge(all_valuable_features, source_target_valuable_features);
   append_valuable_feature(all_valuable_features, *result->owner().create_feature("version", ""), result->owner());

   auto process_one = [&](const feature* f) {
      if (f->attributes().free || f->attributes().no_checks) {
         // copy all
         const std::string& name = f->name();
         for(auto i = target_props.find(name); i != target_props.end(); i = target_props.find(i, name)) {
            result->join(*i);
            std::advance(i, 1);
         }
      } else {
         if (const feature* tpf = target_props.find(f->name().c_str(), f->value().c_str()))
            result->join(target_props.owner().clone_feature(*tpf));
      }
   };

   for (const feature* f : all_valuable_features)
      process_one(f);

   return result;
}

}
