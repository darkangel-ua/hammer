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
         const feature* find(const char* name) const; // find first occurrence 
         feature_set* join(const feature_set& rhs) const;
         feature_set* clone() const;
         void add_propagated(const feature_set& v);

      private:
         feature_registry* fr_;
         features_t features_;
   };
}