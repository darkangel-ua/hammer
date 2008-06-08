#pragma once

#include <antlr3parser.h>
#include <string>
#include "call_resolver.h"

namespace hammer
{
   class engine;
   namespace details
   {
      struct hammer_parser_context
      {
         hammer_parser_context() : error_count_(0), current_arg_(0) {}
         engine* engine_;
         unsigned long error_count_;
         call_resolver::const_iterator current_rule_;
         int current_arg_;
         void (*base_displayRecognitionError)(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
      };

      void on_enter_rule(pANTLR3_PARSER parser, pANTLR3_UINT8 rule_name);
      void on_rule_argument(pANTLR3_PARSER parser);
      bool argument_is_string(pANTLR3_PARSER parser);
      bool argument_is_string_list(pANTLR3_PARSER parser);
      bool argument_is_feature(pANTLR3_PARSER parser);
      bool argument_is_requirements(pANTLR3_PARSER parser);
      bool argument_is_project_requirements(pANTLR3_PARSER parser);
   }
}
