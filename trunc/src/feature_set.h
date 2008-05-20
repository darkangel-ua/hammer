#pragma once

#include <vector>
#include <boost/noncopyable.hpp>

namespace hammer
{
   class feature;
   class feature_registry;

   class feature_set : public boost::noncopyable
   {
      public:
         typedef std::vector<feature*> features_t;
         typedef features_t::const_iterator const_iterator;

         feature_set(feature_registry* fr);
         void insert(feature* f);
         void insert(const char* name, const char* value);
         const_iterator begin() const { return features_.begin(); }
         const_iterator end() const { return features_.end(); }
         const feature* get(const char* name) const; // throw if not found
         const feature* find(const char* name, const char* value) const;
//         const feature* find(const char* name) const; // find first occurrence 
         const_iterator find(const char* name) const; // find first occurrence 
         const_iterator find(const_iterator from, const char* name) const; // find next occurrence 
         feature_set* join(const feature_set& rhs) const;
         void join(const feature_set& v);
         feature_set* clone() const;
         void add_propagated(const feature_set& rhs);

      private:
         feature_registry* fr_;
         features_t features_;

         void join_impl(feature_set* lhs, const feature_set& rhs) const;
   };
}