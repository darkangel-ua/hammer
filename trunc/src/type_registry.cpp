#include "stdafx.h"
#include "type_registry.h"
#include "type.h"

using namespace std;

namespace hammer{

   type_registry::type_registry()
   {

   }
   
   type_registry::~type_registry()
   {

   }

   const type* type_registry::resolve_from_name(const std::string& name) const
   {
      types_t::const_iterator i = types_.find(name);
      if (i != types_.end())
         return &*i;
      else
         throw runtime_error("Can't find type with name '" + name + "'");
   }

   const type* type_registry::resolve_from_target_name(const pstring& name, const feature_set&) const
   {
      string s_name(name.to_string());
      for(types_t::const_iterator i = types_.begin(), last = types_.end(); i != last; ++i)
      {
         string::size_type p = s_name.rfind(i->suffix().c_str());
         if (p != string::npos)
            return &*i;
      }

      throw std::runtime_error("Can't resolve type from target name '" + name.to_string() + '\'');
   }

   const type* type_registry::resolve_from_suffix(const char* first, const char* last) const
   {
      types_by_suffix_t::const_iterator i = types_by_suffix_.find(string(first, last));
      if (i != types_by_suffix_.end())
         return i->second;
      else
         throw runtime_error("Can't find type with suffix '" + string(first, last) + "'");
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