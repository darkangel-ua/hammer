tree grammar hammer_walker;

options
{ 
	language = C; 
 	tokenVocab = hammer;
 	ASTLabelType = pANTLR3_BASE_TREE;
}

@header
{
	#include "../hammer_walker_impl.h"
}

project	:	 ^(PROJECT project_def meta_target*) ;

meta_target : lib | exe ;
project_def
	:
	^(PROJECT_DEF ID) 
	{ hammer_make_project(PARSER->super, $ID.text->chars); }
	;
	
lib
		@init { void* mt; }
 	:	^(LIB ID { mt = hammer_add_lib_meta_target(PARSER->super, $ID.text->chars); } ^(SOURCES sources[mt]?)) 
		
	;
exe
		@init { void* mt; }
 	:	^(EXE ID { mt = hammer_add_exe_meta_target(PARSER->super, $ID.text->chars); } ^(SOURCES sources[mt]?)) 
		
	;

sources[void* mt] :	(ID {hammer_add_target_to_mt(PARSER->super, mt, $ID.text->chars); })+ ;

