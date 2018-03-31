#pragma once
#include <string>
#include <iosfwd>
#include <hammer/parscore/token.h>

namespace hammer { namespace parscore {

class identifier : public token {
   public:
      using token::token;
};

}}
