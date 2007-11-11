grammar hammer;

@lexer::members { boolean some_id_input; }

options { language = Java; output = AST; k = 2; }
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
meta_target :	lib | exe;
project_def :	'project' ID ';' -> ^(PROJECT_DEF ID);
lib 	:	'lib' ID (':' sources?)? ';' -> ^(LIB ID ^(SOURCES sources?)) ;
exe 	:	'exe' ID (':' sources?)? ';' -> ^(EXE ID ^(SOURCES sources?)) ;
sources :	ID+ ;

jam_some_construct :	jam_rule_invoke_top | jam_var_stmt | jam_if | jam_rule;
jam_rule_invoke_top 	:	 ID jam_lol ';' ;
jam_lol	:	jam_list (':' jam_list)* ;
jam_list :  jam_arg* ;
jam_arg 	:	ID | jam_var_expand | '[' jam_func ']' ;
jam_func :  jam_arg jam_lol ;	
jam_var_expand : '$(' ID ('[' ID ']')? (':' ('D' | ('J' '=' ID)))?')';
jam_var_stmt :	'local'? ID (('=' | '?=' | '+=') jam_list)? ';' ;
jam_expr :  jam_arg ;
jam_if :	'if' jam_if_expr jam_block ('else' jam_block)?;
jam_if_expr 
	: jam_if_logical_ops (('=' | '!=') jam_if_logical_ops)*;
jam_if_logical_ops 
	: jam_if_in (('&&' | '|' | '||') jam_if_in)*;
jam_if_in 
	: ('!' jam_if_expr) | ('(' jam_if_expr ')') | (jam_arg ('in' jam_arg)*);

jam_block :	 '{' jam_some_construct* '}' ;
jam_rule : 'rule' ID ('(' jam_rule_args_def? ')')? jam_block ;

jam_rule_args_def :	 jam_rule_arg_def (':' jam_rule_arg_def)* ;
jam_rule_arg_def 	:	 ID ('*' | '?')? ;

ASSIGN :	'=' ;
ID @init{ some_id_input = false; } :	(('a'..'z' | 'A'..'Z' | ('0'..'9') | '.' | '-' | 
		'_' | '@' | '/' | '\\' | '<' | '>' | '*' | ({input.LA(1)!=' '}? '(')) { some_id_input = true; } | ({some_id_input==true && input.LA(1)!= ' '}? ')') )+  |
		JAM_STRING ; 

fragment 
JAM_STRING 	: '"' ('\\"' | ~('"' | '\n' | '\r'))* '"' ;

JAM_COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')  { $channel = HIDDEN; };
WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; };


