grammar hammer;

options { language = Java; output = AST; }
tokens
{
   PROJECT;
   PROJECT_DEF;
   LIB;
   EXE;
   SOURCES;
}

//project	:	(project_def | meta_target | jam_some_construct)*  -> ^(PROJECT project_def meta_target*);
project	:	jam_some_construct*;
//meta_target :	lib | exe;
//project_def :	'project' ID ';' -> ^(PROJECT_DEF ID);
//lib 	:	'lib' ID (':' sources?)? ';' -> ^(LIB ID ^(SOURCES sources?)) ;
//exe 	:	'exe' ID (':' sources?)? ';' -> ^(EXE ID ^(SOURCES sources?)) ;
//sources :	ID+ ;

jam_some_construct :	jam_rule_invoke_top | jam_var_stmt | jam_if | jam_for | jam_rule | jam_module;
jam_rule_invoke_top 	:	 ID jam_lol ';' ;
jam_lol	:	jam_list (':' jam_list)* ;
jam_list :  jam_arg* ;
jam_arg 	:	ID | '[' jam_func ']' ;
jam_func :  jam_arg jam_lol ;	
jam_var_stmt :	'local'? ID (('=' | '?=' | '+=') jam_list)? ';' ;
jam_expr :  jam_arg ;
jam_module : 'module' jam_list jam_block ;
jam_for : 'for' 'local'? ID 'in' jam_list jam_block ;
jam_if :	'if' jam_if_expr jam_block ('else' jam_block)?;
jam_if_expr 
	: jam_if_logical_ops (('=' | '!=') jam_if_logical_ops)*;
jam_if_logical_ops 
	: jam_if_in (('&&' | '|' | '||') jam_if_in)*;
jam_if_in 
	: ('!' jam_if_expr) | ('(' jam_if_expr ')') | (jam_arg ('in' jam_list)*);

jam_block :	 ('{' jam_some_construct* '}') | jam_if ;
jam_rule : 'rule' ID ('(' jam_rule_args_def? ')')? jam_block ;

jam_rule_args_def :	 jam_rule_arg_def (':' jam_rule_arg_def)* ;
jam_rule_arg_def 	:	 (ID ('*' | '?')?)+ ;

ASSIGN :	'=' ;
ID  : ID_BEGIN | JAM_STRING ; 

//fragment
//ID_BEGIN 
//	:   'a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_' | '@' | '/' | '\\' | '<' | '>' | '*' | '=' | '(' | ')' | '$' | '[' | ']';
fragment
ID_BEGIN 
	: (~('\n' | '\r' | ' '))+ ;
fragment 
JAM_STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

JAM_COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')  { $channel = HIDDEN; };
WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };


