#include "stdafx.h"
#include "feature_registry.h"
#include <boost/checked_delete.hpp>
#include "feature_set.h"
#include "feature.h"
#include <list>

using namespace std;

namespace hammer{

   struct feature_registry::impl_t
   {
      impl_t(pool* p) : pstring_pool_(p) {}
      ~impl_t();

      typedef std::map<std::string, feature_def> defs_t;
      typedef std::list<feature_set*> defs_list_t;
      typedef std::list<feature*> features_list_t;
      
      feature_def* find_def(const char* name);

      pool* pstring_pool_;
      defs_t defs_;
      defs_list_t defs_list_;
      features_list_t features_list_;
   };
   
   feature_def* feature_registry::impl_t::find_def(const char* name)
   {
      defs_t::iterator i = defs_.find(string(name));
      if (i == defs_.end())
         return NULL;
      else
         return &i->second;
   }

   feature_registry::impl_t::~impl_t()
   {
      for(defs_list_t::iterator i = defs_list_.begin(), last = defs_list_.end(); i != last; ++i)
         delete *i;

      for(features_list_t::iterator i = features_list_.begin(), last = features_list_.end(); i != last; ++i)
         delete *i;
   }

   typedef feature_registry::impl_t impl_t;

   feature_registry::feature_registry(pool* p) : impl_(new impl_t(p))
   {
   }

   feature_registry::~feature_registry()
   {
      delete impl_;
   }

   feature_set* feature_registry::make_set()
   {
      auto_ptr<feature_set> r(new feature_set(this));
      impl_->defs_list_.push_back(r.get());

      return r.release();
   }

   void feature_registry::add_def(const feature_def& def)
   {
      pair<impl_t::defs_t::iterator, bool> p = impl_->defs_.insert(make_pair(def.name(), def));
      if (!p.second)
         throw std::runtime_error("feature_def with name '" + def.name() + "' already registered");
   }

   feature* feature_registry::create_feature(const char* name, const char* value) const
   {
      auto_ptr<feature> f(new feature(impl_->find_def(name), pstring(*impl_->pstring_pool_, value)));
      impl_->features_list_.push_back(f.get());
      
      return f.release();
   }

   void feature_registry::add_defaults(feature_set* s) const
   {
      typedef impl_t::defs_t::const_iterator iter;
      for(iter i = impl_->defs_.begin(), last = impl_->defs_.end(); i != last; ++i)
      {
         if (!i->second.attributes().incidental &&
             !i->second.attributes().free &&
             !i->second.attributes().no_defaults &&
             s->find(i->first.c_str()) == s->end())
         {
            s->join(create_feature(i->first, i->second.get_default()));
         }
      }
   }

   feature_def& feature_registry::get_def(const std::string& name)
   {
      feature_def* result = impl_->find_def(name.c_str());
      if (result == NULL)
         throw runtime_error("Unknown feature '" + string(name) + "'");
      else
         return *result;
   }

   const feature_def* feature_registry::find_def(const char* feature_name) const
   {
      return impl_->find_def(feature_name);
   }
}
