grammar hammer;

options { language = C; output = AST; }
tokens {
RULE_CALL;
NULL_ARG;
STRING_ARG;
STRING_LIST;
FEATURE_LIST;
FEATURE;
REQUIREMENTS_DECL;
CONDITIONAL_FEATURES;
CONDITION;
PROJECT_REQUIREMENTS;
SOURCES_DECL;
}

@parser::preincludes
{
        #include "../hammer_parser_context.h"
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
                  | feature_list -> ^(FEATURE_LIST feature_list)
                  | { argument_is_sources(PARSER) }?=> sources_decl -> ^(SOURCES_DECL sources_decl);
string_list : string+ -> ^(STRING_LIST string+);
feature_list : feature+;
project_requirements : string requirements -> ^(PROJECT_REQUIREMENTS string ^(REQUIREMENTS_DECL requirements));
requirements : (feature | conditional_features)+;
string_arg  : string -> ^(STRING_ARG string);
feature_arg : feature -> feature;
conditional_features : { is_conditional_feature(PARSER) }?=> condition condition_result -> ^(CONDITIONAL_FEATURES condition condition_result);
condition  : feature (',' feature)* -> ^(CONDITION feature+);
// COLON needed for is_conditional_feature so we put it into token stream
condition_result : COLON feature;
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
string  : ID;
sources_decl  : (string | rule_invoke)+ ;
rule_invoke : '[' { on_nested_rule_enter(PARSER); } rule_impl { on_nested_rule_leave(PARSER); }']' -> rule_impl;

ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '=' | '/' | '*')+  | STRING { LEXSTATE->type = _type; {pANTLR3_COMMON_TOKEN t = LEXER->emit(LEXER); ++t->start, --t->stop; t->type = _type;} };
COLON   : ':';
fragment 
STRING  : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
