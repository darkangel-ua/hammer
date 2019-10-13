#pragma once
#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>

namespace hammer {

typedef boost::filesystem::path location_t;

struct location_equal_to : std::binary_function<location_t, location_t, bool> {
   bool operator()(const location_t& lhs,
                   const location_t& rhs) const
   {
      return lhs.string() == rhs.string();
   }
};

class wcpath : boost::filesystem::path {
   public:
      using path::path;

      location_t& to_location() { return *this; }
      const location_t& to_location() const { return *this; }
};

}

namespace std {

template<>
struct hash<boost::filesystem::path> {
   size_t operator()(const boost::filesystem::path& p) const {
      return boost::filesystem::hash_value(p);
   }
};

}
