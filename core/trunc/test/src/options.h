#if !defined(h_d73fde8f_1206_4c59_9a1a_764b2ba6e15e)
#define h_d73fde8f_1206_4c59_9a1a_764b2ba6e15e

#include <map>
#include <string>
#include <boost/filesystem/path.hpp>

class options : public std::map<std::string, std::string>
{
   public:
      options(const boost::filesystem::path& from_file);

      bool exists(const std::string& key) const
      {
         return find(key) != end();
      }

      void parse_and_add(const std::string& line, const boost::filesystem::path& from_file);
};


#endif //h_d73fde8f_1206_4c59_9a1a_764b2ba6e15e
