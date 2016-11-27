#if !defined(h_5a04546f_8ebb_4a2b_925b_9da5fd7285a2)
#define h_5a04546f_8ebb_4a2b_925b_9da5fd7285a2

#include <hammer/parscore/source_location.h>
#include <string>
#include <iosfwd>

struct ANTLR3_COMMON_TOKEN_struct;

namespace hammer{ namespace parscore{

class identifier
{
   public:
      identifier(source_location lok,
                 unsigned int length) 
         : lok_(lok),
           v_(NULL),
           length_(length),
           no_lok_(false)
      {}

      identifier(const char* v);
      identifier(const ANTLR3_COMMON_TOKEN_struct* v);
      identifier();

      bool operator < (const identifier& rhs) const;
      bool operator == (const identifier& rhs) const;
      const char* begin() const;
      const char* end() const;
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

std::ostream& operator << (std::ostream& os, const identifier& v);

}}

#endif
