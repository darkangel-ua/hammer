#include "stdafx.h"
#include <hammer/core/type.h>
#include <boost/format.hpp>
#include <algorithm>
#include <cassert>

using namespace std;

namespace hammer{

type::type(const type_tag& tag, const std::string& suffix)
   : tag_(tag), suffixes_(1, suffix), base_(NULL)
{

}

type::type(const type_tag& tag, const std::string& suffix, const type& base)
   : tag_(tag), suffixes_(1, suffix), base_(&base)
{

}

type::type(const type_tag& tag, const suffixes_t& suffixes)
   : tag_(tag), suffixes_(suffixes), base_(NULL)
{

}

type::type(const type_tag& tag, const suffixes_t& suffixes, const type& base)
   : tag_(tag), suffixes_(suffixes), base_(&base)
{

}

bool type::equal(const type& rhs) const
{
   return tag() == tag() && 
          suffixes().size() == rhs.suffixes().size() && 
          std::equal(suffixes().begin(), suffixes().end(), rhs.suffixes().begin());
}

bool type::equal_or_derived_from(const type& rhs) const
{ 
   assert(owner_ != NULL && "To perform this operation owner_ must be not NULL.");

   if (owner_ != rhs.owner_)
      return equal(rhs);
   
   if (this == &rhs)
      return true;
   else
      if (base() != NULL)
         return base()->equal_or_derived_from(rhs);
      else
         return false;
}

const std::string& type::suffix_for(const std::string& s) const
{
   for(type::suffixes_t::const_iterator i = suffixes_.begin(), last = suffixes_.end(); i != last; ++i)
   {
      string::size_type p = s.rfind(i->c_str());
      if (p != string::npos && 
         p + i->size() == s.size())
         return *i;
   }

   throw std::runtime_error((boost::format("Type '%s' can't find suffix '%s'.") % tag_.name() % s).str());
}

std::auto_ptr<type> type::clone(const type_registry& tr) const
{
   if (base() != NULL && base()->owner_ != &tr)
      throw std::logic_error("[type] Can't clone because base type is not registered in owner.");

   std::auto_ptr<type> result(base() == NULL ? new type(tag(), suffixes()) : new type(tag(), suffixes(), *base()));
   result->owner_ = &tr;
   return result;
}

}