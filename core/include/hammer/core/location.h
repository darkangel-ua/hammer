#if !defined(h_c13718fb_d5e6_44da_86bb_873e99b9c82a)
#define h_c13718fb_d5e6_44da_86bb_873e99b9c82a

#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>

namespace hammer
{
   typedef boost::filesystem::path location_t;
   
   struct location_equal_to : std::binary_function<location_t, location_t, bool>
   {
      bool operator()(const location_t& lhs, const location_t& rhs) const
      {
         return lhs.string() == rhs.string();
      }
   };
}

namespace std
{
    template<> struct hash<boost::filesystem::path>
    {
        size_t operator()(const boost::filesystem::path& p) const
        {
            return boost::filesystem::hash_value(p);
        }
    };
}

#endif //h_c13718fb_d5e6_44da_86bb_873e99b9c82a
