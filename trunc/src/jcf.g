grammar jcf;

options { language = Java; output = AST; }
tokens{ TARGET; ATTRIBUTES; TARGETS; TYPE_ATTR; FEATURES_ATTR; FEATURE; }

target 	: ID ('[' (attribute ';' )*']')* ('{' target* '}') -> ^(TARGET ^(ATTRIBUTES attribute*) ^(TARGETS target*));
attribute 
	: type | features ;
type 	: 'type' '=' ID -> ^(TYPE_ATTR ID);	
features : 'features' '=' feature+ -> ^(FEATURES_ATTR feature+);
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
	
sources : ID ;
ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+  | STRING;

fragment 
STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
