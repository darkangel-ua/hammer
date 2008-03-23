#pragma once
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include "pstring.h"

namespace hammer
{
   class type;
   class feature_set;

   class type_registry : public boost::noncopyable
   {
      public:
         type_registry();
         const type& resolve_from_name(const std::string& name) const;
         const type& resolve_from_name(const type& t) const;
         const type* resolve_from_suffix(const char* first, const char* last) const;
         const type* resolve_from_suffix(const std::string& suffix) const;
         const type* resolve_from_target_name(const pstring& name, const feature_set& fs) const;
         const type* resolve(const type* t, const feature_set& fs) const;
         void insert(std::auto_ptr<type>& t);
         ~type_registry();

      private:
         typedef boost::ptr_map<const std::string/*name*/, type> types_t;
         typedef std::multimap<std::string, type*> types_by_suffix_t;

         types_t types_;
         types_by_suffix_t types_by_suffix_;

   };
}