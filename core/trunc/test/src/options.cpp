#include "stdafx.h"
#include "options.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/actor/insert_at_actor.hpp>

using namespace boost::spirit;
namespace fs = boost::filesystem;

void options::parse_and_add(const std::string& line, const boost::filesystem::path& from_file)
{
   std::string key, value;
   parse(line.begin(), 
         line.end(), 
          ch_p("#") >> *space_p >> "options:" >> +space_p >> 
          list_p(((+(anychar_p - '='))[assign_a(key)] >> 
                  !('=' >> (+(anychar_p - ' '))[assign_a(value)]))[insert_at_a(*this, key, value)]
             [assign_a(value, std::string())],
         +space_p));
}

options::options(const boost::filesystem::path& from_file)
{
   fs::ifstream f(from_file);
   std::string line;
   while(getline(f, line))
   {
      if (!line.empty() && line[0] == '#')
         parse_and_add(line, from_file);
   }
}

