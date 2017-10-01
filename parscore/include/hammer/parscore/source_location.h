#if !defined(h_885e1bfd_edcb_4c2e_a805_f246b6d45c50)
#define h_885e1bfd_edcb_4c2e_a805_f246b6d45c50

#include <cstddef>
#include <string>

struct ANTLR3_COMMON_TOKEN_struct;

namespace hammer{namespace parscore{

class identifier;
class source_location
{
	  friend class identifier;
   public:
      source_location() : antlr_token_(NULL) {}
      source_location(const ANTLR3_COMMON_TOKEN_struct* antlr_token)
         : antlr_token_(antlr_token)
      {}

      bool valid() const { return antlr_token_ != NULL; }
      std::string full_source_name() const;
      unsigned line() const;

   private:
      const ANTLR3_COMMON_TOKEN_struct* antlr_token_;
};

}}

#endif
