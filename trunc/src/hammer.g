grammar hammer;

options { language = C; output = AST; }
tokens {
RULE_CALL;
NULL_ARG;
STRING_LIST;
FEATURE_LIST;
FEATURE;
}

project :        rules;
rules :  rule*;
rule    :       ID rule_args ';' -> ^(RULE_CALL ID rule_args);
rule_args  : rule_posible_args? maybe_arg*;
maybe_arg 
        : ':' rule_posible_args -> rule_posible_args
        | ':' -> ^(NULL_ARG)
        ;               
rule_posible_args : string_list -> ^(STRING_LIST string_list)
                  | feature_list -> ^(FEATURE_LIST feature_list);
string_list : string+;
feature_list : feature+;
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
string  : ID ;

ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '=' | '/')+  | STRING;

fragment 
STRING  : '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
