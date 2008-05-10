#include "stdafx.h"
#include "type_registry.h"
#include "type.h"
#include "types.h"

using namespace std;

namespace hammer{

   type_registry::type_registry()
   {

   }
   
   type_registry::~type_registry()
   {

   }

   const type& type_registry::resolve_from_name(const std::string& name) const
   {
      types_t::const_iterator i = types_.find(name);
      if (i != types_.end())
         return *i->second;
      else
         throw runtime_error("Can't find type with name '" + name + "'");
   }

   const type& type_registry::resolve_from_name(const type& t) const
   {
      return resolve_from_name(t.name());
   }

//    const type* type_registry::resolve(const type* t, const feature_set& fs) const
//    {
//       if (t == &resolve_from_name(types::EXE.name()))
//          return t;
// 
//       if (t == &resolve_from_name(types::LIB.name()))
//          return &resolve_from_name(types::SHARED_LIB.name());
// 
//       return t;
//    }

   const type* type_registry::resolve_from_target_name(const pstring& name, const feature_set&) const
   {
      string s_name(name.to_string());
      for(types_t::const_iterator i = types_.begin(), last = types_.end(); i != last; ++i)
      {
         string::size_type p = s_name.rfind(i->second->suffix().c_str());
         if (p != string::npos && 
             p + i->second->suffix().size() == s_name.size())
            return i->second;
      }

      return 0;
   }

   const type* type_registry::resolve_from_suffix(const char* first, const char* last) const
   {
      return resolve_from_suffix(string(first, last));
   }

   const type* type_registry::resolve_from_suffix(const std::string& suffix) const
   {
      types_by_suffix_t::const_iterator i = types_by_suffix_.find(suffix);
      if (i != types_by_suffix_.end())
         return i->second;
      else
         throw runtime_error("Can't find type with suffix '" + suffix + "'");
   }

   void type_registry::insert(std::auto_ptr<type>& t)
   {
      pair<types_t::iterator, bool> i = types_.insert(t->name(), t.get());
      if (i.second)
      {
         types_by_suffix_.insert(make_pair(t->suffix(), t.get()));
         t.release();
         return;
      }
      else
         throw std::runtime_error("[type_registry] Can't add type '" + t->name() + "' twice");
   }
}