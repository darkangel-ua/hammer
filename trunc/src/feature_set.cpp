#include "stdafx.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"
#include <iterator>
#include "sources_decl.h"

using namespace std;

namespace hammer{

   feature_set::feature_set(feature_registry* fr) : fr_(fr)
   {
   }

   feature_set& feature_set::join(const char* name, const char* value)
   {
      return join(fr_->create_feature(name, value));
   }

   feature_set& feature_set::join(feature* f)
   {
      if (!f->attributes().free)
      {
         iterator i = find(f->name());
         if (i != end())
         {
            if ((**i).value() != f->value())
            {
               *i = f;
               if (f->attributes().composite)
                  f->def().expand_composites(f->value().to_string(), this);
            }
         }
         else
         {
            features_.push_back(f);
            if (f->attributes().composite)
               f->def().expand_composites(f->value().to_string(), this);
         }
      }
      else
         features_.push_back(f);
      
      return *this;
   }

   const feature& feature_set::get(const char* name_) const
   {
      const_iterator f = find(name_);
      if (f == features_.end())
         throw runtime_error("feature '" + string(name_) + "not founded");
      
      return **f;
   }
   
   feature_set::iterator feature_set::deconstify(const_iterator i)
   {
      iterator result = features_.begin();
      std::advance(result, std::distance<feature_set::const_iterator>(features_.begin(), i));
      return result;
   }

   feature_set::iterator feature_set::find(const char* name)
   {
      return deconstify(static_cast<const feature_set*>(this)->find(name));
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

   feature_set::iterator feature_set::find(iterator from, const char* name)
   {
      return deconstify(static_cast<const feature_set*>(this)->find(from, name));
   }

   const feature* feature_set::find(const char* name, const char* value) const
   {
      for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
         if ((**i).def().name() == name && (**i).value() == value)
            return *i;

      return 0;
   }

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

   void feature_set::copy_propagated(const feature_set& v)
   {
      for(const_iterator i = v.begin(), last = v.end(); i != last; ++i)
      {
         if ((*i)->attributes().propagated)
            join(*i);
      }
   }

   feature_set::const_iterator feature_set::find(const feature& f) const
   {
      if (f.attributes().free)
      {
         for(const_iterator i = find(f.name()), last = end(); i != last;)
            if ((**i).value() == f.value())
               return i;
            else
               i = find(++i, f.name());
         
         return end();
      }
      else
         return find(f.name());
   }

   void set_path_data(feature_set* f, const basic_meta_target* t)
   {
      typedef feature_set::iterator iter;
      for(iter i = f->begin(), last = f->end(); i != last; ++i)
      {
         if ((**i).attributes().path)
            (**i).get_path_data().target_ = t;
      }
   }

   void extract_sources(sources_decl& result, const feature_set& fs)
   {
      // FIXME: need refactor this two blocks
      feature_set::const_iterator i = fs.find("source");
      while(i != fs.end())
      {
         result.push_back((**i).get_dependency_data().source_);
         i = fs.find(++i, "source");
      }

      i = fs.find("library");
      while(i != fs.end())
      {
         result.push_back((**i).get_dependency_data().source_);
         i = fs.find(++i, "library");
      }
   }

   void extract_uses(sources_decl& result, const feature_set& fs)
   {
      // FIXME: need refactor this block
      feature_set::const_iterator i = fs.find("use");
      while(i != fs.end())
      {
         result.push_back((**i).get_dependency_data().source_);
         i = fs.find(++i, "use");
      }
   }

   bool feature_set::operator == (const feature_set& rhs) const
   {
      return this == &rhs;
   }
}