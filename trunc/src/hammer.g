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
rule    : ID { on_enter_rule(PARSER, $ID.text->chars); } rule_args WS+ ';' rule_tail -> ^(RULE_CALL ID rule_args);
rule_tail : WS+
          | ;
rule_args  : (rule_posible_args { on_rule_argument(PARSER); })? (maybe_arg { on_rule_argument(PARSER); })*;
maybe_arg 
        : WS+ ':' rule_posible_args -> rule_posible_args
        | WS+ ':' -> ^(NULL_ARG)
        ;               
rule_posible_args 
                  : { argument_is_string(PARSER) }?=> string_arg
                  | { argument_is_project_requirements(PARSER) }?=> project_requirements 
                  | { argument_is_string_list(PARSER) }?=> string_list -> ^(STRING_LIST string_list)
                  | { argument_is_feature(PARSER) }?=> feature_arg
                  | { argument_is_requirements(PARSER) }?=> requirements -> ^(REQUIREMENTS_DECL requirements)
                  | feature_list -> ^(FEATURE_LIST feature_list)
                  | { argument_is_sources(PARSER) }?=> sources_decl -> ^(SOURCES_DECL sources_decl);
string_list : (WS+ string)+ -> string+;
feature_list : (WS+ feature)+ -> feature+;
project_requirements : WS+ string requirements -> ^(PROJECT_REQUIREMENTS string ^(REQUIREMENTS_DECL requirements));
requirements : (r_feature | r_conditional_features)+;
string_arg  : WS+ string -> ^(STRING_ARG string);
feature_arg : WS+ feature -> feature;
r_feature : WS+ feature -> feature;
r_conditional_features 	: WS+ conditional_features -> conditional_features;
conditional_features : condition ':' condition_result -> ^(CONDITIONAL_FEATURES condition condition_result);
condition  : feature (',' feature)* -> ^(CONDITION feature+);
condition_result : feature;
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
string  : ID ;
sources_decl  : string_list ;
ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '=' | '/')+  | STRING;

fragment 
STRING  : '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) ; //{ $channel = HIDDEN; };
