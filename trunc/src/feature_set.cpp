#include "stdafx.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"

using namespace std;

namespace hammer{

   feature_set::feature_set(feature_registry* fr) : fr_(fr)
   {
   }

   void feature_set::insert(const char* name, const char* value)
   {
      features_.push_back(fr_->create_feature(name, value));
   }

   void feature_set::insert(feature* f)
   {
      features_.push_back(f);
   }

   const feature* feature_set::get(const char* name_) const
   {
      const_iterator f = find(name_);
      if (f == features_.end())
         throw runtime_error("feature '" + string(name_) + "not founded");
      
      return *f;
   }

   feature_set::const_iterator feature_set::find(const char* name) const
   {
      return find(features_.begin(), name);
   }

   feature_set::const_iterator feature_set::find(const_iterator from, const char* name) const
   {
      for(features_t::const_iterator i = from, last = features_.end(); i != last; ++i)
         if ((**i).def().name() == name )
            return i;

      return features_.end();
   }

   const feature* feature_set::find(const char* name, const char* value) const
   {
      for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
         if ((**i).def().name() == name && (**i).value() == value)
            return *i;

      return 0;
   }

/*
   const feature* feature_set::find(const char* name) const
   {
      for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
         if ((**i).def().name() == name )
            return *i;

      return 0;
   }
*/

   void feature_set::join_impl(feature_set* lhs, const feature_set& rhs) const
   {
      lhs->features_.insert(lhs->features_.end(), rhs.features_.begin(), rhs.features_.end());
   }

   feature_set* feature_set::join(const feature_set& rhs) const
   {
      assert(fr_);
      feature_set* result = fr_->make_set();
      result->features_ = features_;
      join_impl(result, rhs);
      return result;
   }

   void feature_set::join(const feature_set& rhs)
   {
      join_impl(this, rhs);
   }

   feature_set* feature_set::clone() const
   {
      feature_set* result = fr_->make_set();
      result->features_ = features_;
      return result;
   }

   void feature_set::add_propagated(const feature_set& v)
   {
      for(const_iterator i = v.begin(), last = v.end(); i != last; ++i)
      {
         if ((*i)->attributes().propagated)
            insert(*i);
      }
   }

}