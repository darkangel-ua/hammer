#pragma once

#include <boost/noncopyable.hpp>
#include "pool.h"
#include "pstring.h"

namespace hammer
{
   class type;
   class feature_set;

   class basic_target : public boost::noncopyable
   {
      public:
         basic_target(const pstring& name, const type* t, const feature_set* f) : name_(name), type_(t), features_(f) {};
         const pstring& name() const { return name_; }
         const hammer::type* type() const { return type_; }
         const feature_set& features() const { return *features_; }
         void* operator new (size_t size, pool& p) { return p.malloc(size); }
         void operator delete (void* m, pool& p) {};
         virtual ~basic_target(){};
      
      private:
         const hammer::type* type_;
         pstring name_;
         const feature_set* features_;
   };
}