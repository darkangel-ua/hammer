grammar hammer;

options { language = Java; output = AST;}

rules :	 rule*;
rule 	:	ID rule_args ';' ;
rule_args  : rule_posible_args? (':' rule_posible_args?)* ;
rule_posible_args : (feature_of_feature_list | string_or_string_list)+;
feature_of_feature_list : feature_list | feature ;	 
feature_list : feature feature+;
feature  : '<' ID '>' ID ;
string_or_string_list :	( string_list) | (string);
string_list 
	:	string string+;
string 	: ID ;

ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+  | STRING;

fragment 
STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
