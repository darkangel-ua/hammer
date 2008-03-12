grammar jcf;

options { language = Java; output = AST; }

target 	: ID ('[' (attribute ';' )*']') ('{' *target '}')* ';' ;
attribute 
	: type | features ;
type 	: 'type' '=' ID ;	
features : 'features' '=' feature+ ;
feature  : '<' ID '>' ID ;
	
sources : ID ;
ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+  | STRING;

fragment 
STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
