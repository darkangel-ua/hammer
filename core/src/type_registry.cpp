#include "stdafx.h"
#include <stdexcept>
#include <hammer/core/type_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>

using namespace std;

namespace hammer{

   type_registry::type_registry()
   {

   }
   
   type_registry::~type_registry()
   {

   }

   std::string::size_type rfind(const std::string& where, const std::string& what)
   {
      if (where.empty() || what.empty())
         return std::string::npos;

      const char* where_first = &where[0];
      const char* where_last = &*(where.end() - 1);
      const char* what_first = what.c_str();
      const char* what_last = what.c_str() + what.size() - 1;
      
      while(where_first <= where_last && 
            what_first <= what_last)
      {
         if (*where_last != *what_last)
            break;

         --where_last;
         --what_last;
      }

      return where_first <= where_last ? where_last - where_first + 1
                                       : std::string::npos;
   }

   // FIXME: need totally rewrite this logic in very efficiently way because it calls many times
   const target_type* type_registry::resolve_from_target_name(const std::string& name) const
   {
      for(types_t::const_iterator i = types_.begin(), last = types_.end(); i != last; ++i)
      {
         // skip types with empty suffix
         if (i->second->suffixes().empty())
            continue;
         
         for(target_type::suffixes_t::const_iterator j = i->second->suffixes().begin(), j_last = i->second->suffixes().end(); j != j_last; ++j)
         {
            string::size_type p = rfind(name, j->suffix_);//s_name.rfind(j->c_str());
            if (p != string::npos && 
                p + j->suffix_.size() == name.size())
               return i->second.get();
         }
      }

      return 0;
   }

   const target_type& type_registry::soft_resolve_from_target_name(const std::string& name) const
   {
      const target_type* t = resolve_from_target_name(name);
      if (!t)
         return get(types::UNKNOWN);
      else
         return *t;
   }
   const target_type& type_registry::hard_resolve_from_target_name(const std::string& name) const
   {
      const target_type* t = resolve_from_target_name(name);
      if (!t)
         throw std::runtime_error("Can't resolve type from target name '" + name + "'.");
      
      return *t;
   }

   const target_type* type_registry::resolve_from_suffix(const char* first, const char* last) const
   {
      return resolve_from_suffix(string(first, last));
   }

   const target_type* type_registry::resolve_from_suffix(const std::string& suffix) const
   {
      types_by_suffix_t::const_iterator i = types_by_suffix_.find(suffix);
      if (i != types_by_suffix_.end())
         return i->second;
      else
         throw runtime_error("Can't find type with suffix '" + suffix + "'");
   }

   const target_type* type_registry::find(const type_tag& tag) const
   {
      types_t::const_iterator i = types_.find(tag);
      if (i != types_.end())
         return i->second.get();
      else
         return nullptr;
   }

   const target_type& type_registry::get(const type_tag& tag) const
   {
      const target_type* result = find(tag);
      if (result)
         return *result;
      else
         throw runtime_error("Can't find type with tag '" + tag.name() + "'");
   }

   const target_type& type_registry::insert(const target_type& a_t)
   {
      std::unique_ptr<target_type> t{a_t.clone(*this)};
      auto* raw_t = t.get();
      type_tag tag{t->tag()};
      pair<types_t::iterator, bool> i = types_.insert({tag, std::move(t)});
      if (i.second)
      {
         if (!raw_t->suffixes().empty())
         {
            for(target_type::suffixes_t::const_iterator i = raw_t->suffixes().begin(), last = raw_t->suffixes().end(); i != last; ++i)
               types_by_suffix_.insert(make_pair(i->suffix_, raw_t));
         }

         return *raw_t;
      }
      else
         throw std::runtime_error("[type_registry] Can't add type '" + raw_t->tag().name() + "' twice");
   }
}
