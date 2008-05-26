#pragma once

#include <vector>
#include <boost/noncopyable.hpp>

namespace hammer
{
   class feature;
   class feature_registry;
   class feature_def; 
   class feature_set : public boost::noncopyable
   {
      public:
         typedef std::vector<feature*> features_t;
         typedef features_t::const_iterator const_iterator;
         typedef features_t::iterator iterator;

         feature_set(feature_registry* fr);
         const_iterator begin() const { return features_.begin(); }
         const_iterator end() const { return features_.end(); }
         iterator end() { return features_.end(); }
         const feature* get(const char* name) const; // throw if not found
         const feature* find(const char* name, const char* value) const;
         const_iterator find(const char* name) const; // find first occurrence 
         iterator find(const std::string& name) { return find(name.c_str()); }
         iterator find(const char* name); // find first occurrence 
         const_iterator find(const_iterator from, const char* name) const; // find next occurrence 
         iterator find(iterator from, const char* name); // find next occurrence 
         feature_set* join(const feature_set& rhs) const;
         feature_set& join(feature* f);
         feature_set& join(const char* name, const char* value);
         void join(const feature_set& v);
         feature_set* clone() const;
         void copy_propagated(const feature_set& rhs);
         size_t size() const { return features_.size(); }

      private:
         feature_registry* fr_;
         features_t features_;

         void join_impl(feature_set* lhs, const feature_set& rhs) const;
         iterator deconstify(const_iterator i);
         void add_composites(const feature_def& def);
   };
}