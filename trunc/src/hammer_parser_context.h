#pragma once

#include <antlr3parser.h>

namespace hammer
{
   class engine;
   namespace details
   {
      struct hammer_parser_context
      {
         hammer_parser_context() : error_count_(0) {}
         engine* engine_;
         unsigned long error_count_;

         void (*base_displayRecognitionError)(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
      };

      void on_rule(pANTLR3_PARSER parser);
   }
}
