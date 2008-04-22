#pragma once

#include <boost/noncopyable.hpp>
#include "pool.h"
#include "pstring.h"
#include "build_node.h"

namespace hammer
{
   class type;
   class feature_set;
   class main_target;

   class basic_target : public boost::noncopyable
   {
      public:
         basic_target(const main_target* mt, const pstring& name, 
                      const type* t, const feature_set* f) 
                     : mtarget_(mt), name_(name), 
                       type_(t), features_(f)
         {};

         const pstring& name() const { return name_; }
         const hammer::type& type() const { return *type_; }
         const feature_set& features() const { return *features_; }
         const hammer::main_target* mtarget() const { return mtarget_; }
         
         virtual boost::intrusive_ptr<build_node> generate() = 0;
         void* operator new (size_t size, pool& p) { return p.malloc(size); }
         void operator delete (void* m, pool& p) {};
         virtual ~basic_target(){};
      
      private:
         const main_target* mtarget_;
         const hammer::type* type_;
         pstring name_;
         const feature_set* features_;
         std::vector<basic_target*> dependencies_;
   };
}