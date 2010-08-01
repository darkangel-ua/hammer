#if !defined(h_885e1bfd_edcb_4c2e_a805_f246b6d45c50)
#define h_885e1bfd_edcb_4c2e_a805_f246b6d45c50

#include <cstddef>

struct ANTLR3_COMMON_TOKEN_struct;

namespace hammer{namespace parscore{

class source_location
{
   public:
      source_location() : antlr_token_(NULL) {}
      source_location(const ANTLR3_COMMON_TOKEN_struct* antlr_token)
         : antlr_token_(antlr_token)
      {}

      // FIXME: bad name
      const char* begin() const;
      bool valid() const { return antlr_token_ != NULL; }

   private:
      const ANTLR3_COMMON_TOKEN_struct* antlr_token_;
};

}}

#endif //h_885e1bfd_edcb_4c2e_a805_f246b6d45c50
