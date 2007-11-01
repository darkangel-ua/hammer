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
         feature_set(feature_registry* fr);
         typedef std::vector<feature*> features_t;
         void insert(feature* f);
         void insert(const char* name, const char* value);

      private:
         feature_registry* fr_;
         features_t features_;
   };
}