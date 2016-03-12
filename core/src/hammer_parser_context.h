#if !defined(h_0cb5b3c6_38b2_4139_bccb_93ae6eefc038)
#define h_0cb5b3c6_38b2_4139_bccb_93ae6eefc038

#include <antlr3parser.h>
#include <string>
#include <stack>
#include <map>
#include <hammer/core/call_resolver.h>
#include <hammer/core/feature_def.h>

namespace hammer
{
   class engine;
   namespace details
   {
      
      struct non_buffered_token_stream;
      struct hammer_parser_context
      {
         struct rule_context
         {
            rule_context() : arg_(0), 
                             in_feature_feature_rule_(false),
                             new_feature_(NULL)
            {}

            int arg_;
            call_resolver::const_iterator rule_;
            bool in_feature_feature_rule_;
            feature_def* new_feature_;
         };
         
         typedef std::map<std::string, feature_def> new_features_t;

         hammer_parser_context() : error_count_(0) {}
         engine* engine_;
         unsigned long error_count_;
         rule_context rule_context_;
         std::stack<rule_context> rule_contexts_;
         non_buffered_token_stream* token_stream_;
         new_features_t new_features_;
         
         void (*base_displayRecognitionError)(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
      };

      void on_enter_rule(pANTLR3_PARSER parser, pANTLR3_UINT8 rule_name);
      void on_rule_argument(pANTLR3_PARSER parser);
      void on_nested_rule_enter(pANTLR3_PARSER parser);
      void on_nested_rule_leave(pANTLR3_PARSER parser);
      void on_string_list_element(pANTLR3_PARSER parser, pANTLR3_UINT8 id_);
      bool argument_is_string(pANTLR3_PARSER parser);
      bool argument_is_string_list(pANTLR3_PARSER parser);
      bool argument_is_feature(pANTLR3_PARSER parser);
      bool argument_is_requirements(pANTLR3_PARSER parser);
      bool argument_is_project_requirements(pANTLR3_PARSER parser);
      bool argument_is_sources(pANTLR3_PARSER parser);
      bool is_conditional_feature(pANTLR3_PARSER parser);
      bool is_path_element(pANTLR3_PARSER parser);
      bool is_path_slash(pANTLR3_PARSER parser);
      bool is_trailing_slash(pANTLR3_PARSER parser);
      bool is_head_slash(pANTLR3_PARSER parser);
      bool is_dependency_feature(pANTLR3_PARSER parser);

      void enter_sources_decl(pANTLR3_PARSER parser);
      void leave_sources_decl(pANTLR3_PARSER parser);
      void enter_rule_invoke(pANTLR3_PARSER parser);
      void leave_rule_invoke(pANTLR3_PARSER parser);
   }
}

#endif //h_0cb5b3c6_38b2_4139_bccb_93ae6eefc038
