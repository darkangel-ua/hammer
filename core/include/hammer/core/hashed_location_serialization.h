#pragma once
#include <hammer/core/hashed_location.h>

namespace boost { namespace serialization {

template<typename Archive>
void serialize(Archive & ar,
               hammer::hashed_location& v,
               const unsigned int version) {
   if (Archive::is_saving::value) {
      std::string s(v.location().string());
      ar & s;
   } else {
      std::string tmp;
      ar & tmp;
      v = hammer::hashed_location(tmp);
   }
}

}}
