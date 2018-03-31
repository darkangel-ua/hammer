#pragma once
#include <hammer/parscore/source_location.h>
#include <string>
#include <iosfwd>

struct ANTLR3_COMMON_TOKEN_struct;

namespace hammer { namespace parscore {

class token {
   public:
      token(source_location lok,
            unsigned int length)
         : lok_(lok),
           v_(NULL),
           length_(length),
           no_lok_(false)
      {}

      token(const char* v);
		token(const ANTLR3_COMMON_TOKEN_struct* v);
      token();

      bool operator < (const token& rhs) const;
      bool operator == (const token& rhs) const;
		bool operator == (const char* rhs) const;
      std::string to_string() const;
      // FIXME: posible bug if no_lok_ == true
      source_location start_lok() const { return lok_; }
      bool valid() const { return (no_lok_ && v_) || lok_.valid(); }

   private:
      // FIXME: we need ether source_location pointed on unknown location
      //        or boost::varian<source_location, const char*>
      source_location lok_;
      const char* v_;

      unsigned int length_:31;
      bool no_lok_ : 1;
};

std::ostream&
operator << (std::ostream& os, const token& v);

}}
