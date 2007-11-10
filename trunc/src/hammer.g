grammar hammer;

options { language = C; output = AST; }

tokens
{
   PROJECT;
   PROJECT_DEF;
   LIB;
   EXE;
   SOURCES;
}
project	:	(project_def | meta_target | jam_some_construct)*  -> ^(PROJECT project_def meta_target*);
meta_target :	lib | exe;
project_def :	'project' ID ';' -> ^(PROJECT_DEF ID);
lib 	:	'lib' ID (':' sources?)? ';' -> ^(LIB ID ^(SOURCES sources?)) ;
exe 	:	'exe' ID (':' sources?)? ';' -> ^(EXE ID ^(SOURCES sources?)) ;
sources :	ID+ ;

jam_some_construct :	jam_rule_invoke_top | jam_var_stmt | jam_if;
jam_rule_invoke_top 	:	 ID jam_lol ';' ;
jam_lol	:	jam_list (':' jam_list)* ;
jam_list :  jam_arg* ;
jam_arg 	:	ID | jam_var_expand | '[' jam_func ']' ;
jam_func :  jam_arg jam_lol ;	
jam_var_expand : '$' '(' ID ('[' ID ']')? (':' ('D' | ('J' '=' ID)))?')';
jam_var_stmt :	'local'? ID (('=' | '?=' | '+=') jam_list)? ';' ;
jam_expr :  jam_arg ;
jam_if :	'if' jam_cond_expr jam_block ;
jam_cond_expr 	:	('!' jam_cond) | jam_cond_and_or | ('(' jam_cond ')');
jam_cond_and_or 	:	 jam_cond (('&&' | '||') jam_cond)* ;
jam_cond	:	jam_expr ((ASSIGN | '!=' | '<' | '<=' | '>' | '>=' | 'in') jam_expr)* ;
                  

jam_block :	 '{' jam_some_construct* '}' ;
jam_rule : 'rule' ID ('(' jam_rule_args_def? ')')? jam_block ;

jam_rule_args_def :	 jam_rule_arg_def (':' jam_rule_arg_def)* ;
jam_rule_arg_def 	:	 ID ('*' | '?')? ;

ASSIGN :	'=' ;
ID	:	('a'..'z' | 'A'..'Z' | ('0'..'9') | '.' | '-' | '_' | '@' | '/' | '\\' | '<' | '>')+  | JAM_STRING ; 

fragment 
JAM_STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

JAM_COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')  { $channel = HIDDEN; };
WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };


