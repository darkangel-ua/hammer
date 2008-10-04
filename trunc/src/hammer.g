grammar hammer;

options { language = C; output = AST; }
tokens {
RULE_CALL;
NULL_ARG;
STRING_ARG;
STRING_LIST;
FEATURE_SET;
FEATURE_SET_ARG;
FEATURE;
FEATURE_ARG;
REQUIREMENTS_DECL;
CONDITIONAL_FEATURES;
CONDITION;
PROJECT_REQUIREMENTS;
SOURCES_DECL;
SOURCE_DECL;
TARGET_PATH;
TARGET_NAME;
SOURCE_DECL_EXPLICIT_TARGET;
}

@parser::preincludes
{
        #include "../hammer_parser_context.h"
        using namespace hammer::details;
}

@lexer::preincludes
{
        #include "../non_buffered_token_stream.h"
        using namespace hammer::details;
}

project : WS* rules -> rules;
rules :  rule*;
rule    : rule_impl ';' -> rule_impl;
rule_impl : ID { on_enter_rule(PARSER, $ID.text->chars); } rule_args -> ^(RULE_CALL ID rule_args);
rule_args  : (rule_posible_args { on_rule_argument(PARSER); })? (maybe_arg { on_rule_argument(PARSER); })*;

maybe_arg 
        : ':' rule_posible_args -> rule_posible_args
        | ':' -> ^(NULL_ARG)
        ;         
rule_posible_args 
                  : { argument_is_string(PARSER) }?=> string_arg
                  | { argument_is_project_requirements(PARSER) }?=> project_requirements 
                  | { argument_is_string_list(PARSER) }?=> string_list
                  | { argument_is_feature(PARSER) }?=> feature_arg
                  | { argument_is_requirements(PARSER) }?=> requirements -> ^(REQUIREMENTS_DECL requirements)
                  | feature_set_arg
                  | { argument_is_sources(PARSER) }?=> sources_decl;
string_list : (ID { on_string_list_element(PARSER, $ID.text->chars); })+ -> ^(STRING_LIST ID+);
feature_set_arg : feature_set -> ^(FEATURE_SET_ARG feature_set);
feature_set : feature+ -> ^(FEATURE_SET feature+);
project_requirements : ID requirements -> ^(PROJECT_REQUIREMENTS ID ^(REQUIREMENTS_DECL requirements));
requirements : (feature | conditional_features)+;
string_arg  : ID -> ^(STRING_ARG ID);
feature_arg : feature -> ^(FEATURE_ARG feature);
conditional_features : { is_conditional_feature(PARSER) }?=> condition condition_result -> ^(CONDITIONAL_FEATURES condition condition_result);
condition  : feature (',' feature)* -> ^(CONDITION feature+);
// COLON needed for is_conditional_feature so we put it into token stream
condition_result : COLON feature;
feature       : { is_dependency_feature(PARSER) }?=> '<' ID '>' source_decl -> ^(FEATURE ID source_decl)
              | '<' ID '>' ID -> ^(FEATURE ID ID);
sources_decl : { enter_sources_decl(PARSER); } sources_decl_impl { leave_sources_decl(PARSER); } -> ^(SOURCES_DECL sources_decl_impl) ;
sources_decl_impl  : (source_decl | rule_invoke)+ ;
rule_invoke   : { enter_rule_invoke(PARSER); } '[' { on_nested_rule_enter(PARSER); } rule_impl { on_nested_rule_leave(PARSER); }']' { leave_rule_invoke(PARSER); } -> rule_impl;

source_decl : source_decl_impl -> ^(SOURCE_DECL source_decl_impl);
source_decl_impl : target_path target_name target_features -> ^(TARGET_PATH target_path) target_name target_features;
target_path : head_slash? ID path_element* trail_slash?;
target_name : path_slash path_slash ID -> ^(TARGET_NAME ID)
            | -> ^(TARGET_NAME NULL_ARG);
path_element : { is_path_element(PARSER) }?=> path_slash ID;
target_features : path_slash feature (path_slash feature)* -> ^(FEATURE_SET feature+)
                | -> ^(FEATURE_SET NULL_ARG);            
path_slash : { is_path_slash(PARSER) }?=> SLASH ;
trail_slash : { is_trailing_slash(PARSER) }?=> SLASH;
head_slash : { is_head_slash(PARSER) }?=> SLASH; 
SLASH : { is_lexing_sources_decl(LEXER) }?=> '/';
ID            : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | { !is_lexing_sources_decl(LEXER) }?=> '/' | '_'| '=' | '*')+  
	      | STRING { LEXSTATE->type = _type; {pANTLR3_COMMON_TOKEN t = LEXER->emit(LEXER); ++t->start, --t->stop; t->type = _type;} };
COLON   : ':';
fragment 
STRING  : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
