#include "stdafx.h"
#include <hammer/core/target_type.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/type_registry.h>
#include <boost/format.hpp>
#include <algorithm>
#include <cassert>

using namespace std;

namespace hammer{

std::vector<const feature*>
target_type::make_valuable_features(const suffixes_t& suffixes,
                                    const prefixes_t& prefixes)
{
   std::vector<const feature*> result;

   for (auto& s : suffixes) {
      if (s.condition_)
         append_valuable_features(result, *s.condition_);
   }

   for (auto& p : prefixes) {
      if (p.condition_)
         append_valuable_features(result, *p.condition_);
   }

   return result;
}

target_type::target_type(const type_tag& tag, const suffix_def& suffix, const prefix_def& prefix)
   : tag_(tag),
     suffixes_(1, suffix),
     prefixes_(1, prefix),
     base_(NULL),
     valuable_features_(make_valuable_features(suffixes_, prefixes_))
{

}

target_type::target_type(const type_tag& tag, const suffix_def& suffix, const target_type& base, const prefix_def& prefix)
   : tag_(tag),
     suffixes_(1, suffix),
     prefixes_(1, prefix),
     base_(&base),
     valuable_features_(make_valuable_features(suffixes_, prefixes_))
{

}

target_type::target_type(const type_tag& tag, const suffixes_t& suffixes, const prefixes_t& prefixes)
   : tag_(tag),
     suffixes_(suffixes),
     prefixes_(prefixes),
     base_(NULL),
     valuable_features_(make_valuable_features(suffixes_, prefixes_))
{

}

target_type::target_type(const type_tag& tag, const suffixes_t& suffixes, const target_type& base, const prefixes_t& prefixes)
   : tag_(tag),
     suffixes_(suffixes),
     prefixes_(prefixes),
     base_(&base),
     valuable_features_(make_valuable_features(suffixes_, prefixes_))
{

}

bool target_type::equal(const target_type& rhs) const
{
   return tag() == tag() && 
          suffixes().size() == rhs.suffixes().size() && 
          std::equal(suffixes().begin(), suffixes().end(), rhs.suffixes().begin());
}

bool target_type::suffix_def::operator == (const suffix_def& rhs) const
{
   return suffix_ == rhs.suffix_ && *condition_ == *rhs.condition_;
}

bool target_type::equal_or_derived_from(const target_type& rhs) const
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

bool target_type::equal_or_derived_from(const type_tag& rhs) const
{
   assert(owner_ != NULL && "To perform this operation owner_ must be not NULL.");

   return equal_or_derived_from(owner_->get(rhs));
}

bool target_type::operator == (const target_type& rhs) const
{
   assert(owner_ != NULL && "To perform this operation owner_ must be not NULL.");
   if (owner_ != rhs.owner_)
      return equal(rhs);
   
   return this == &rhs;
}

const std::string& target_type::suffix_for(const std::string& s, const feature_set& environment) const
{
   for(target_type::suffixes_t::const_iterator i = suffixes_.begin(), last = suffixes_.end(); i != last; ++i)
   {
      string::size_type p = s.rfind(i->suffix_.c_str());
      if (p != string::npos && 
         p + i->suffix_.size() == s.size())
         return i->suffix_;
   }

   throw std::runtime_error((boost::format("Type '%s' can't find suffix '%s'.") % tag_.name() % s).str());
}

const std::string& target_type::suffix_for(const feature_set& environment) const
{
   for(target_type::suffixes_t::const_iterator i = suffixes_.begin(), last = suffixes_.end(); i != last; ++i)
   {
      if (i->condition_ == NULL || environment.contains(*i->condition_))
         return i->suffix_;
   }

   throw std::runtime_error((boost::format("Type '%s' can't find suffix in environment[FIXME]") % tag_.name()).str());
}

static std::string empty_prefix;
const std::string& target_type::prefix_for(const feature_set& environment) const
{
   for(target_type::prefixes_t::const_iterator i = prefixes_.begin(), last = prefixes_.end(); i != last; ++i)
   {
      if (i->condition_ == NULL || environment.contains(*i->condition_))
         return i->suffix_;
   }

   return empty_prefix;
}

std::auto_ptr<target_type> target_type::clone(const type_registry& tr) const
{
   if (base() != NULL && base()->owner_ != &tr)
      throw std::logic_error("[type] Can't clone because base type is not registered in owner.");

   std::auto_ptr<target_type> result(base() == NULL ? new target_type(tag(), suffixes()) : new target_type(tag(), suffixes(), *base(), prefixes()));
   result->owner_ = &tr;
   return result;
}

}
