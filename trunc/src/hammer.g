grammar hammer;

options { language = Java; output = AST;}

rules :	 rule*;
rule 	:	ID rule_args ';' ;
rule_args  : rule_posible_args? (':' rule_posible_args?)* ;
rule_posible_args : string_list | 
		    feature_list | 
		    string feature_list;
		    
feature_list : feature+;
feature  : '<' ID '>' ID ;
string_list : string+;
string 	: ID ;

ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+  | STRING;

fragment 
STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
