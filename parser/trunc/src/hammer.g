grammar hammer;

options{ 
//   language = C; 
   output = AST;
//   ASTLabelType = pANTLR3_BASE_TREE;
}

tokens{
HAMFILE;
PROJECT_DEF;
TARGET_DECL;
TARGET_REF;
TARGET_NAME;
PARAMS;
PARAMETER;
EMPTY_EXPRESSION;
EXPRESSION;
LIST_OF;
PATH_LIKE_SEQ;
FEATURE_SET;
FEATURE;
}

@parser::preincludes
{
   #include "../hammer_parser_context.h"
}

hamfile       : project_def? target_decl* -> ^(HAMFILE project_def? target_decl*);
project_def   : WS* 'project' params ';' -> ^(PROJECT_DEF 'project' params);
target_decl   : WS* ID WS+ ID target_params WS* ';' -> ^(TARGET_DECL ID ID target_params);
target_params : (WS* ':' parameter)* -> ^(PARAMS parameter*);
params        : parameter (WS* ':' parameter)* -> ^(PARAMS parameter+);
parameter     : expression
              | -> EMPTY_EXPRESSION;
expression    : WS* feature_set -> feature_set
              | list_of;
feature_set   : feature (WS+ feature)? -> ^(FEATURE_SET feature+);
feature       : '<' ID '>' feature_value -> ^(FEATURE ID feature_value);
feature_value : path_like_seq
              | '(' target_ref ')' -> target_ref;
path_like_seq : '/' path_like_seq_impl -> ^(PATH_LIKE_SEQ SLASH path_like_seq_impl)
	      |	path_like_seq_impl -> ^(PATH_LIKE_SEQ path_like_seq_impl);
path_like_seq_impl : ID ('/' ID)? '/'? -> ID+;
target_ref : path_like_seq target_ref_impl -> ^(TARGET_REF path_like_seq target_ref_impl);
target_ref_impl : target_props -> TARGET_NAME target_props
                | target_name_seq target_props?; 
target_name_seq : '//' ID -> ^(TARGET_NAME ID);
target_props : (WS* '/' feature)+ -> ^(FEATURE_SET feature+);
list_of : WS* list_of_impl (WS+ list_of_impl)* -> ^(LIST_OF list_of_impl+);
list_of_impl : path_like_seq
             | target_ref;
//sources_like_list : ID+;

SLASH : '/';
ID : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '=' | '*')+  
	      | STRING ;//{ LEXSTATE->type = _type; {pANTLR3_COMMON_TOKEN t = LEXER->emit(LEXER); ++t->start, --t->stop; t->type = _type;} };
COLON : ':';
fragment 
STRING : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ); //{ $channel = HIDDEN; };
