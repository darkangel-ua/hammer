#include "stdafx.h"
#include <hammer/core/fs_argument_writer.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/build_node.h>

namespace hammer{

void fs_argument_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   const feature_set& build_request = node.build_request();
   bool is_first = true;
   for(patterns_t::const_iterator i = patterns_.begin(), last = patterns_.end(); i != last; ++i)
   {
      if (i->first->size() == 1 &&
          (**i->first->begin()).attributes().generated)
      {
         feature_set::const_iterator f = build_request.find((**i->first->begin()).name());
         if (f != build_request.end() &&
             (**f).value() == (**i->first->begin()).value())
         {
            if (is_first)
               is_first = false;
            else
               output << ' ';

            output << i->second;
         }
      }
      else
         if (build_request.contains(*i->first))
         {
            if (is_first)
               is_first = false;
            else
               output << ' ';

            output << i->second;
         }
   }
}

fs_argument_writer& fs_argument_writer::add(const feature_set* pattern, const std::string& what_write)
{
   patterns_.push_back(std::make_pair(pattern, what_write));
   return *this;
}

fs_argument_writer& fs_argument_writer::add(const std::string& pattern, const std::string& what_write)
{
   add(parse_simple_set(pattern, *feature_registry_), what_write);
   return *this;
}

fs_argument_writer* fs_argument_writer::clone() const
{
   return new fs_argument_writer(*this);
}

std::vector<feature_ref>
fs_argument_writer::valuable_features() const
{
   std::vector<feature_ref> result;

   for (auto& p : patterns_)
      append_valuable_features(result, *p.first);

   return result;
}

}
