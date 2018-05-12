#pragma once
#include <cstddef>
#include <string>

struct ANTLR3_COMMON_TOKEN_struct;

namespace hammer { namespace parscore {

class identifier;
class source_location {
	  friend class token;
   public:
      source_location() : antlr_token_(NULL) {}
      source_location(const ANTLR3_COMMON_TOKEN_struct* antlr_token)
         : antlr_token_(antlr_token)
      {}

      bool valid() const { return antlr_token_ != NULL; }
      std::string full_source_name() const;
      std::string line_content() const;
      unsigned line() const;
      unsigned char_pos() const;

   private:
      const ANTLR3_COMMON_TOKEN_struct* antlr_token_;
};

}}
