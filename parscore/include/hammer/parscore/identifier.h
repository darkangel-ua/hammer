#pragma once
#include <string>
#include <iosfwd>
#include <hammer/parscore/token.h>

namespace hammer { namespace parscore {

class identifier : public token {
   public:
      // all these contructor can be replaced by using token::token but msvc-12.0 can't handle this
      identifier(source_location lok,
                 unsigned int length)
         : token(lok, length)
      {}

      identifier(const char* v) : token(v) {}
      identifier(const ANTLR3_COMMON_TOKEN_struct* v) : token(v) {}
      identifier() {}
};

}}
