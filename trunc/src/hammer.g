grammar hammer;

options { language = C; output = AST; }
tokens {
RULE_CALL;
NULL_ARG;
STRING_LIST;
FEATURE_LIST;
FEATURE;
REQUIREMENTS_DECL;
CONDITIONAL_FEATURES;
CONDITION;
}

@parser::preincludes
{
        #include "../hammer_parser_context.h"
        using namespace hammer::details;
}

project :        rules;
rules :  rule*;
rule    :       WS* ID { on_enter_rule(PARSER, $ID.text->chars); } rule_args WS+ ';' -> ^(RULE_CALL ID rule_args);
rule_args  : rule_posible_args? maybe_arg*;
maybe_arg 
        : WS+ ':' rule_posible_args -> rule_posible_args
        | WS+ ':' { on_rule_argument(PARSER); }-> ^(NULL_ARG)
        ;               
rule_posible_args 
@init{ 
	on_rule_argument(PARSER); 
	bool is_feature = argument_is_feature(PARSER); 
} 
                  : string_list -> ^(STRING_LIST string_list)
                  | { is_feature }? feature
                  | { argument_is_requirements(PARSER) }? requirements -> ^(REQUIREMENTS_DECL requirements)
                  | feature_list -> ^(FEATURE_LIST feature_list);
string_list : (WS+ string)+ -> string+;
feature_list : (WS+ feature)+ -> feature+;
requirements : (r_feature | r_conditional_features)+;
r_feature : WS+ feature -> feature;
r_conditional_features 	: WS+ conditional_features -> conditional_features;
conditional_features : condition ':' condition_result -> ^(CONDITIONAL_FEATURES condition condition_result);
condition  : feature (',' feature)* -> ^(CONDITION feature+);
condition_result : feature;
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
string  : ID ;

ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '=' | '/')+  | STRING;

fragment 
STRING  : '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) ; //{ $channel = HIDDEN; };
