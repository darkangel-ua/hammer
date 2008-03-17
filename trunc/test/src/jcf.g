grammar jcf;

options { language = C; output = AST; }
tokens{ TARGET; ATTRIBUTES; TARGETS; TYPE_ATTR; FEATURES_ATTR; FEATURE; }

jsf_file : targets ;
targets : target+ -> ^(TARGETS target+)
        | ;
target 	: ID ('[' attributes ']')* '{' target* '}' -> ^(TARGET ID attributes* target*);

attributes : (attribute ';' )+ -> ^(ATTRIBUTES attribute+)
           | ;
attribute 
	: type 
	| features ;
	
type 	: 'type' '=' ID -> ^(TYPE_ATTR ID);	
features : 'features' '=' feature+ -> ^(FEATURES_ATTR feature+);
feature  : '<' ID '>' ID -> ^(FEATURE ID ID);
	
sources : ID ;
ID  :   ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+  | STRING;

fragment 
STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };
