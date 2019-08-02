#include <list>
#include <map>
#include <cassert>
#include <cstdlib>
#include <unordered_map>
#include <boost/checked_delete.hpp>
#include <boost/tokenizer.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/make_unique.hpp>
#include <boost/algorithm/string/join.hpp>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/feature_value_ns.h>

using std::string;
using std::unique_ptr;
using std::pair;
using namespace boost::multi_index;

namespace hammer {

   struct feature_registry::impl_t {
      impl_t(std::shared_ptr<feature_registry> parent)
         : parent_(std::move(parent))
      {}

      ~impl_t();

      struct generic_less_comparator
      {
         static const bool equality_result = false;

         template<typename T1, typename T2>
         bool operator()(const T1& lhs, const T2& rhs) const { return lhs < rhs; }
      };

      struct generic_greater_equal_comparator
      {
         static const bool equality_result = true;

         template<typename T1, typename T2>
         bool operator()(const T1& lhs, const T2& rhs) const { return !(lhs < rhs || lhs == rhs); }
      };

      struct subfeature_storage_item
      {
         const feature_def* feature_def_;
         std::shared_ptr<subfeature> subfeature_;
      };
      
      struct subfeature_find_data
      {
         subfeature_find_data(const feature_def& fdef, 
                              const subfeature_def& sdef,
                              const std::string& value)
                              : fdef_(fdef), sdef_(sdef), value_(value)
         {}

         const feature_def& fdef_;
         const subfeature_def& sdef_;
         const std::string& value_;
      };

      struct subfeature_storage_item_comparator
      {
         bool operator()(const subfeature_storage_item& lhs, 
                         const subfeature_storage_item& rhs) const
         {
            if (lhs.feature_def_ != rhs.feature_def_)
               return lhs.feature_def_ <  rhs.feature_def_;

            if (lhs.subfeature_.get() != rhs.subfeature_.get())
               return lhs.subfeature_.get() < rhs.subfeature_.get();

            return false;
         }

         template<typename Comparator>
         bool compare_impl(const subfeature_storage_item& lhs, 
                           const subfeature_find_data& rhs,
                           Comparator comparator) const
         {
            if (lhs.feature_def_ != &rhs.fdef_)
               return comparator(lhs.feature_def_, &rhs.fdef_);
            
            if (&lhs.subfeature_->definition() != &rhs.sdef_)
               return comparator(&lhs.subfeature_->definition(), &rhs.sdef_);

            if (lhs.subfeature_->value() != rhs.value_)
               return comparator(lhs.subfeature_->value(),  rhs.value_);

            return false;
         }

         bool operator()(const subfeature_storage_item& lhs, 
                         const subfeature_find_data& rhs) const 
         { 
            return compare_impl(lhs, rhs, generic_less_comparator()); 
         }            

         bool operator()(const subfeature_find_data& lhs, 
                         const subfeature_storage_item& rhs) const 
         { 
            return compare_impl(rhs, lhs, generic_greater_equal_comparator()); 
         }            
      };

      struct find_feature_data
      {
         find_feature_data(const feature_def& def, 
                           const string& value,
                           const feature::subfeatures_t& subfeatures) 
                           : def_(def), value_(value), subfeatures_(subfeatures)
         {}

         const feature_def& def_;
         const string& value_;
         const feature::subfeatures_t subfeatures_;
      };

      struct feature_storage_comparator
      {
         bool operator()(const feature& lhs,
                         const feature& rhs) const
         {
            if (&lhs == &rhs)
               return false;
            
            if (&lhs.definition() != &rhs.definition())
               return &lhs.definition() < &rhs.definition();
            
            if (lhs.value() != rhs.value())
               return lhs.value() < rhs.value();

            if (lhs.subfeatures().size() != rhs.subfeatures().size())
               return lhs.subfeatures().size() < rhs.subfeatures().size();

            // FIXME: this is due msvc-8.0 debug iterator bug
            if (lhs.subfeatures().empty())
               return false;

            for(feature::subfeatures_t::const_iterator i = lhs.subfeatures().begin(), 
                                                       last = lhs.subfeatures().end(), 
                                                       outer = rhs.subfeatures().begin(); 
                i != last; 
                ++i, ++outer)
             {
                if (*i != *outer)
                   return *i < *outer;
             }

            return false;
         }

         template<typename Comparator>
         bool compare_impl(const feature& lhs,
                           const find_feature_data& rhs,
                           Comparator comparator) const
         {
            if (&lhs.definition() != &rhs.def_)
               return comparator(&lhs.definition(), &rhs.def_);

            if (lhs.value() != rhs.value_)
               return comparator(lhs.value(), rhs.value_);
            
            if (lhs.subfeatures().size() != rhs.subfeatures_.size())
               return comparator(lhs.subfeatures().size(), rhs.subfeatures_.size());
            
            // FIXME: this is due msvc-8.0 debug iterator bug
            if (lhs.subfeatures().empty())
               return false;

            typedef feature::subfeatures_t::const_iterator iter;
            for(iter i = lhs.subfeatures().begin(), 
                     last = lhs.subfeatures().end(), 
                     outer = rhs.subfeatures_.begin(); 
                i != last; 
                ++i, ++outer)
             {
               if (*i != *outer)
                  return comparator(*i, *outer);
             }

            return false;
         }

         bool operator()(const feature& lhs, const find_feature_data& rhs) const 
         {
            return compare_impl(lhs, rhs, generic_less_comparator());
         }

         bool operator()(const find_feature_data& lhs, const feature& rhs) const 
         {
            return compare_impl(rhs, lhs, generic_greater_equal_comparator());
         }
      };
      
      struct feature_storage_key_extractor
      {
         typedef const feature& result_type;
         result_type operator()(const std::shared_ptr<feature>& v) const { return *v; }
      };

      typedef std::map<std::string, std::unique_ptr<feature_def> > defs_t;
      typedef std::list<feature_set*> feature_set_storage_t;
      typedef std::vector<std::unique_ptr<feature>> non_cached_features_t;
      typedef std::unordered_map<std::string, feature_value_ns_ptr> value_namespaces;

      typedef multi_index_container<std::shared_ptr<feature>,
                                    indexed_by<
                                       ordered_unique<feature_storage_key_extractor, feature_storage_comparator> >
                                   > features_t;
      typedef features_t::nth_index<0>::type main_feature_index_t;

      typedef multi_index_container<subfeature_storage_item, 
                                    indexed_by< 
                                       ordered_unique<identity<subfeature_storage_item>, subfeature_storage_item_comparator> >
                                   > subfeatures_t;
      typedef subfeatures_t::nth_index<0>::type main_subfeature_index_t;

      feature_def* find_def(const std::string& name);
      feature* find_feature(const std::string& name, const string& value);
      feature* find_feature(const feature& f, 
                            const subfeature& sf);
      subfeature& create_subfeature(const feature& f, const string& name, const string& value);

      std::shared_ptr<feature_registry> parent_;
      defs_t defs_;
      feature_set_storage_t feature_set_list_;
      features_t features_;
      non_cached_features_t non_cached_features_;
      subfeatures_t subfeatures_;
      value_namespaces value_namespaces_;
   };

   feature_def*
   feature_registry::impl_t::find_def(const std::string& name)
   {
      auto i = defs_.find(name);
      return i == defs_.end() ? nullptr : i->second.get();
   }

   feature*
   feature_registry::impl_t::find_feature(const string& name,
                                          const string& value)
   {
      feature_def* def = find_def(name);
      if (!def)
         return nullptr;

       auto i = features_.get<0>().find(find_feature_data(*def, value, feature::subfeatures_t()));
       if (i == features_.get<0>().end())
          return nullptr;
       else 
          return i->get();
   }

   subfeature&
   feature_registry::impl_t::create_subfeature(const feature& f,
                                               const string& name,
                                               const string& value)
   {
      const subfeature_def* sdef = f.definition().find_subfeature(name);
      if (!sdef)
         throw std::runtime_error("Feature '" + f.name() + "' does not have subfeature '" + name + "'.");
      
      if (!sdef->is_legal_value(f.value(), value))
         throw std::runtime_error("Value '" + value + "' is not a legal value for subfeature '" + name + "'.");

      main_subfeature_index_t::iterator i = subfeatures_.get<0>().find(subfeature_find_data(f.definition(), *sdef, value));
      if (i != subfeatures_.get<0>().end())
         return *i->subfeature_;
      else
      {
         subfeature_storage_item item;
         item.feature_def_ = &f.definition();
         item.subfeature_.reset(new subfeature(*sdef, value));
         subfeature* result = item.subfeature_.get();
         subfeatures_.get<0>().insert(item);
         return *result;
      }
   }

   feature*
   feature_registry::impl_t::find_feature(const feature& f,
                                          const subfeature& sf)
   {
      feature::subfeatures_t subfeatures(f.subfeatures());
      subfeatures.push_back(&sf);
      main_feature_index_t::iterator i = features_.get<0>().find(find_feature_data(f.definition(), f.value(), subfeatures));
      if (i == features_.get<0>().end())
         return nullptr;
      else 
         return i->get();
   }

   feature_registry::impl_t::~impl_t()
   {
      for(feature_set_storage_t::iterator i = feature_set_list_.begin(), last = feature_set_list_.end(); i != last; ++i)
         delete *i;
   }

   typedef feature_registry::impl_t impl_t;

   feature_registry::feature_registry(std::shared_ptr<feature_registry> parent)
      : impl_{new impl_t{std::move(parent)}}
   {
      impl_->value_namespaces_.insert({ string{}, feature_value_ns_ptr{}});
   }

   feature_registry::~feature_registry()
   {
   }

   feature_set*
   feature_registry::make_set() const
   {
      unique_ptr<feature_set> r(new feature_set(const_cast<feature_registry*>(this)));
      impl_->feature_set_list_.push_back(r.get());

      return r.release();
   }

   feature_def&
   feature_registry::add_feature_def(const std::string& name,
                                     const feature_def::legal_values_t& legal_values,
                                     feature_attributes attributes)
   {
      auto i = impl_->find_def(name);
      if (i)
         throw std::runtime_error("Definition for feature '" + name + "' already registered");

      auto r = impl_->defs_.insert({name, unique_ptr<feature_def>(new feature_def{name, legal_values, attributes})});
      return *r.first->second;
   }

   feature*
   feature_registry::simply_create_feature(const feature_def& def,
                                           const std::string& value) const {
      feature* result;

      if (def.attributes().path || 
          def.attributes().dependency ||
          def.attributes().generated)
      {
         unique_ptr<feature> f(new feature(&def, value));
         result = f.get();
         impl_->non_cached_features_.push_back(std::move(f));
      } else {
         result = impl_->find_feature(def.name(), value);
         if (!result) {
            auto f = std::shared_ptr<feature>(new feature{&def, value});
            result = f.get();
            impl_->features_.get<0>().insert(f);
         }
      }
      
      return result;
   }

   feature*
   feature_registry::simply_create_feature(const feature_def& def,
                                           const std::string& value,
                                           const std::function<feature*(const feature_def& def)>& constructor) const {
      feature* result;

      if (def.attributes().path ||
          def.attributes().dependency ||
          def.attributes().generated)
      {
         unique_ptr<feature> f(constructor(def));
         result = f.get();
         impl_->non_cached_features_.push_back(std::move(f));
      } else {
         result = impl_->find_feature(def.name(), value);
         if (!result) {
            auto f = std::shared_ptr<feature>(constructor(def));
            result = f.get();
            impl_->features_.get<0>().insert(f);
         }
      }

      return result;
   }

   feature*
   feature_registry::create_feature(const std::string& name,
                                    const std::string& value,
                                    const std::function<feature*(const feature_def& def)>& constructor) const {
      if (name.empty())
         throw std::runtime_error("Can't create feature without name");

      const feature_def* posible_feature = impl_->find_def(name);
      if (!posible_feature) {
         if (impl_->parent_)
            return impl_->parent_->create_feature(name, value, constructor);
         else
            throw std::runtime_error("There is no definition for feature '" + name + "'");
      }

      return simply_create_feature(*posible_feature, value, constructor);
   }

   feature*
   feature_registry::create_feature(const std::string& name,
                                    const std::string& value,
                                    const project& p) const {
      return create_feature(name, value, [&] (const feature_def& def) {
         return new feature(&def, value, p);
      });
   }

   feature*
   feature_registry::create_feature(const std::string& name,
                                    const std::string& value,
                                    const basic_target& t) const {
      return create_feature(name, value, [&] (const feature_def& def) {
         return new feature(&def, value, t);
      });
   }

   feature*
   feature_registry::create_feature(const std::string& name,
                                    const std::string& value) const
   {
      if (name.empty())
         throw std::runtime_error("Can't create feature without name");

      const feature_def* posible_feature = impl_->find_def(name);
      if (!posible_feature) {
         if (impl_->parent_)
            return impl_->parent_->create_feature(name, value);
         else
            throw std::runtime_error("There is no definition for feature '" + name + "'");
      }

      if (posible_feature->attributes().free ||
          posible_feature->is_legal_value(value))
      {
         return simply_create_feature(*posible_feature, value);
      }

      typedef boost::tokenizer<boost::char_separator<char>, const char*> tokenizer;
      tokenizer tok(value.c_str(), value.c_str() + value.size(), 
                    boost::char_separator<char>("-"));
      tokenizer::const_iterator first = tok.begin(), last = tok.end();
      if (first != last) {
         feature* result = simply_create_feature(*posible_feature, *first);
         const string feature_value = *first;
         ++first;

         for(; first != last; ++first) {
            const subfeature_def* sdef = result->definition().find_subfeature_for_value(feature_value, *first);
            if (!sdef) {
               if (result->attributes().no_checks)
                  return simply_create_feature(*posible_feature, value);
               else
                  throw std::runtime_error("Can't find subfeature with legal value '" + *first + "' for feature <" + name + ">" + result->value());
            } else
               result = create_feature(*result, sdef->name(), *first);
         }

         return result;
      }

      return simply_create_feature(*posible_feature, value);
   }

   feature*
   feature_registry::create_feature(const feature& f,
                                    const string& subfeature_name,
                                    const string& subfeature_value) const
   {
      // we create feature in registry where it was originally registered
      auto feature_def = impl_->find_def(f.name());
      if (!feature_def && impl_->parent_)
         return impl_->parent_->create_feature(f, subfeature_name, subfeature_value);

      subfeature& sf = impl_->create_subfeature(f, subfeature_name, subfeature_value);
      feature* result = impl_->find_feature(f, sf);
      if (!result)
      {
         feature::subfeatures_t new_subfeatures(f.subfeatures());
         new_subfeatures.push_back(&sf);
         auto shared_result = std::shared_ptr<feature>{new feature{&f.definition(), f.value(), new_subfeatures}};
         result = shared_result.get();
         impl_->features_.get<0>().insert(shared_result);
      }

      return result;
   }

   feature_set&
   feature_registry::add_defaults(feature_set& s) const
   {
      if (impl_->parent_)
         impl_->parent_->add_defaults(s);

      for (auto& fd : impl_->defs_) {
         if (!fd.second->attributes().optional &&
             !fd.second->attributes().free &&
             !fd.second->attributes().no_defaults &&
             s.find(fd.first.c_str()) == s.end())
         {
            for (auto& d : fd.second->get_defaults())
               s.join(create_feature(fd.first, d.value_));
         }
      }

      return s;
   }

   feature_def&
   feature_registry::get_def(const std::string& name) {
      return const_cast<feature_def&>(static_cast<const feature_registry*>(this)->get_def(name));
   }

   const feature_def&
   feature_registry::get_def(const std::string& name) const
   {
      feature_def* result = impl_->find_def(name);
      if (!result) {
         if (impl_->parent_)
            return impl_->parent_->get_def(name);
         else
            throw std::runtime_error("There is no feature definition for feature '" + name + "'.");
      } else
         return *result;
   }

   const feature_def*
   feature_registry::find_def(const std::string& name) const
   {
      return const_cast<feature_registry*>(this)->find_def(name);
   }

   feature_def*
   feature_registry::find_def(const std::string& name) {
      auto* result = impl_->find_def(name);
      if (!result && impl_->parent_)
         return impl_->parent_->find_def(name);

      return result;
   }

   const feature_def*
   feature_registry::find_def(const char* feature_name) const
   {
      return impl_->find_def(feature_name);
   }

   const feature_def*
   feature_registry::find_def_from_full_name(const char* feature_name) const
   {
      const feature_def* result = find_def(feature_name);
      if (result)
         return result;
      // FIXME: performance hit
      string feature_name_str(feature_name);
      string::size_type p = feature_name_str.find('-');
      if (p != string::npos)
         return find_def(feature_name_str.substr(0, p).c_str());
      else
         return nullptr;
   }

   feature*
   feature_registry::clone_feature(const feature& f) {
      feature* result;

      if (f.subfeatures().empty())
         result = create_feature(f.name(), f.value());
      else {
         std::vector<string> subfeatures{f.value()};

         std::transform(f.subfeatures().begin(), f.subfeatures().end(),
                        std::back_inserter(subfeatures),
                        [](const subfeature* sf) {
            return sf->value();
         });

         result = create_feature(f.name(), boost::join(subfeatures, "-"));
      }

      if (f.attributes().dependency)
         result->set_dependency_data(f.get_dependency_data().source_, f.get_path_data().project_);

      if (f.attributes().path)
         result->get_path_data() = f.get_path_data();

      return result;
   }

   const feature_value_ns_ptr&
   feature_registry::get_or_create_feature_value_ns(const std::string& ns)
   {
      auto i = impl_->value_namespaces_.find(ns);
      if (i == impl_->value_namespaces_.end())
         return impl_->value_namespaces_.insert({ns, feature_value_ns_ptr(new feature_value_ns(ns))}).first->second;
      else
         return i->second;
   }
}
