#if !defined(h_c13718fb_d5e6_44da_86bb_873e99b9c82a)
#define h_c13718fb_d5e6_44da_86bb_873e99b9c82a

#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>

namespace hammer
{
   typedef boost::filesystem::path location_t;   
}

namespace boost
{
   namespace filesystem
   {
      inline std::size_t hash_value(const path& p)
      {
         return boost::hash_value(p.string());
      }
   }
}

#endif //h_c13718fb_d5e6_44da_86bb_873e99b9c82a
