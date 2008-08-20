#pragma once
#include <stack>
#include <antlr3lexer.h>

namespace hammer
{
   namespace details
   {
      class hammer_lexer_context
      {
         public:
            std::stack<bool> source_lexing_;
            pANTLR3_COMMON_TOKEN_STREAM tstream_;
            pANTLR3_INPUT_STREAM input_;
            pANTLR3_LEXER lexer_;
      };

      bool is_source_lexing(pANTLR3_LEXER lexer);
   }
}