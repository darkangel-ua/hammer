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
project	:	project_def meta_target* -> ^(PROJECT project_def meta_target*);
meta_target :	lib | exe;
project_def :	'project' ID ';' -> ^(PROJECT_DEF ID);
lib 	:	'lib' ID (':' sources?)? ';' -> ^(LIB ID ^(SOURCES sources?)) ;
exe 	:	'exe' ID (':' sources?)? ';' -> ^(EXE ID ^(SOURCES sources?)) ;
sources :	ID+ ;
ID	:	('a'..'z' | 'A'..'Z' | ('0'..'9') | '.' | '-')+  
	;

WS : (' ' |'\n' |'\r' ) { $channel = HIDDEN; } ;

