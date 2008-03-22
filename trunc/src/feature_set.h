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
//        const feature* get(const char* name) const; // throw if not found
         const feature* find(const char* name, const char* value) const;
         feature_set* join(const feature_set& rhs) const;

      private:
         feature_registry* fr_;
         features_t features_;
   };
}