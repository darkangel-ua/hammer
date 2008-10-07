#include "stdafx.h"
#include "type.h"
#include <boost/format.hpp>

using namespace std;

namespace hammer{

type::type(const std::string& name, const std::string& suffix, type* base)
   : name_(name), suffixes_(1, suffix), base_(base)
{

}

type::type(const std::string& name, const suffixes_t& suffixes, type* base)
   : name_(name), suffixes_(suffixes), base_(base)
{

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

   throw std::runtime_error((boost::format("Type '%s' can't find suffix '%s'.") % name() % s).str());
}

}