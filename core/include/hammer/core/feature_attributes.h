#pragma once

namespace hammer {

struct feature_attributes {
   unsigned incidental  : 1;
   unsigned propagated  : 1;
   unsigned free        : 1;
   unsigned optional    : 1;
   unsigned path        : 1;
   unsigned implicit    : 1;
   unsigned dependency  : 1;
   unsigned composite   : 1;
   unsigned no_defaults : 1;
   unsigned no_checks   : 1;
   // if feature has some generated target info
   unsigned generated   : 1;
   // means that lower values can be overriden by higher
   // say, <cxxstd>98 can be overriden by <cxxstd>11
   unsigned ordered     : 1;
};

}
