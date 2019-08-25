#include "stdafx.h"
#include <stdexcept>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <hammer/core/cmdline_builder.h>
#include <hammer/core/argument_writer.h>
#include <hammer/core/feature_set.h>

using namespace std;

namespace hammer{

cmdline_builder::cmdline_builder(const std::string& cmd) : cmd_(cmd)
{
}

void cmdline_builder::add(std::shared_ptr<argument_writer> v)
{
   if (!writers_.insert({v->name(), v}).second)
      throw std::runtime_error("[cmdline_builder] Argument writer '" + v->name() + "' already registered");
}

void cmdline_builder::write(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   using namespace boost::spirit::classic;

   typedef boost::iterator_range<const char*> range_t;
   typedef vector<range_t> arguments_t;

   arguments_t arguments;
   const char* c_str_cmd = cmd_.c_str();
   const char* end_c_str_cmd = c_str_cmd + cmd_.size();
   if (!parse(c_str_cmd, end_c_str_cmd,
              *(anychar_p - str_p("$(")) >> 
                 !list_p(str_p("$(") >> (+(anychar_p - ')'))[push_back_a(arguments)] >> ')', *(anychar_p - "$(")) >> 
              *anychar_p
       ).full)
   {
      throw std::runtime_error("[cmdline_builder] Can't parse command template '" + cmd_ + "'");
   }
   
   for(arguments_t::const_iterator i = arguments.begin(), last = arguments.end(); i != last; ++i)
   {
      if (c_str_cmd + 2 != i->begin())
         output.write(c_str_cmd, static_cast<std::streamsize>(i->begin() - 2 - c_str_cmd));

      string writer(i->begin(), i->end());
      writers_t::const_iterator w = writers_.find(writer);
      if (w == writers_.end())
         throw std::runtime_error("[cmdline_builder] Argument writer with name '" + writer + "' has not been registered");
      
      w->second->write(output, node, environment);
      c_str_cmd = i->end() + 1;
   }

   if (c_str_cmd != end_c_str_cmd)
      output.write(c_str_cmd, static_cast<std::streamsize>(end_c_str_cmd - c_str_cmd));
}

std::vector<feature_ref>
cmdline_builder::valuable_features() const
{
   std::vector<feature_ref> result;

   for(const auto& writer : writers_)
      merge(result, writer.second->valuable_features());

   return result;
}

}
