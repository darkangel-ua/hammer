#include "stdafx.h"
#include "options.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/spirit/include/classic_insert_at_actor.hpp>

using namespace boost::spirit::classic;
namespace fs = boost::filesystem;

void options::parse_and_add(const std::string& line,
                            const boost::filesystem::path& from_file)
{
   std::string key, value;
   parse(line.begin(), 
         line.end(), 
          ch_p("#") >> *space_p >> "options:" >> +space_p >> 
          list_p(((+(anychar_p - '='))[assign_a(key)] >> 
                  !('=' >> (+(anychar_p - ' '))[assign_a(value)]))[insert_at_a(options_, key, value)]
             [assign_a(value, std::string())],
         +space_p));
}

bool options::exists(const std::string& key) const
{
   return options_.find(key) != options_.end();
}

const std::string&
options::operator [](const std::string& key)
{
   auto i = options_.find(key);
   if (i == options_.end())
      throw std::runtime_error("There is no options with key '" + key + "'");

   return i->second;
}

options::options(const boost::filesystem::path& from_file)
{
   fs::ifstream f(from_file);
   std::string line;
   while(getline(f, line)) {
      if (!line.empty() && line[0] == '#')
         parse_and_add(line, from_file);
   }
}

