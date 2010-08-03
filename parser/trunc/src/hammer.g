grammar hammer;

options{ 
   language = C; 
   output = AST;
   ASTLabelType = pANTLR3_BASE_TREE;
}

tokens{
HAMFILE;
EXPLICIT_PROJECT_DEF;
IMPLICIT_PROJECT_DEF;
TARGET_DECL_OR_RULE_CALL;
TARGET_REF;
TARGET_NAME;
EMPTY_TARGET_NAME;
ARGUMENTS;
EXPRESSION;
EMPTY_EXPRESSION;
LIST_OF;
PATH_LIKE_SEQ;
FEATURE_SET;
FEATURE;
PUBLIC_TAG;
CONDITIONAL_FEATURE;
}

@parser::preincludes
{
}

hamfile       : WS* project_def target_decl_or_rule_call* -> ^(HAMFILE project_def? target_decl_or_rule_call*);
project_def   : 'project' arguments WS* ';' WS* -> ^(EXPLICIT_PROJECT_DEF 'project' arguments)
              | -> IMPLICIT_PROJECT_DEF;
target_decl_or_rule_call : target_decl_or_rule_call_impl WS* ';' WS* -> target_decl_or_rule_call_impl;
target_decl_or_rule_call_impl : ID arguments -> ^(TARGET_DECL_OR_RULE_CALL ID arguments);
arguments     : WS+ expression (WS+ ':' argument)* -> ^(ARGUMENTS expression argument*)
              | -> ^(ARGUMENTS);
argument         : WS+ expression -> expression
              | -> EMPTY_EXPRESSION; 
expression    : feature_set -> feature_set
              | list_of ;
feature_set   : feature_set_feature (WS+ feature_set_feature)* -> ^(FEATURE_SET feature_set_feature+);
feature_set_feature : public_tag? feature_set_feature_impl -> ^(FEATURE public_tag? feature_set_feature_impl);
feature_set_feature_impl : (conditional_feature)=> conditional_feature
			 | feature_impl ;
conditional_feature : condition feature -> ^(CONDITIONAL_FEATURE condition feature);
condition : feature condition_impl;
// colon must stay here because if it is not syntactic predicate will not work
condition_impl : (',' feature)* COLON -> feature* COLON;
feature       : feature_impl -> ^(FEATURE feature_impl);
feature_impl : '<' ID '>' feature_value -> ID feature_value;
feature_value : path_like_seq
              | '(' target_ref ')' -> target_ref;
path_like_seq : SLASH path_like_seq_impl -> ^(PATH_LIKE_SEQ SLASH path_like_seq_impl)
	      |	path_like_seq_impl -> ^(PATH_LIKE_SEQ path_like_seq_impl);
path_like_seq_impl : ID ('/' ID)* '/'? -> ID+;
target_ref : path_like_seq target_ref_impl -> ^(TARGET_REF path_like_seq target_ref_impl)
           | public_tag path_like_seq target_ref_impl? -> ^(TARGET_REF public_tag path_like_seq target_ref_impl?);
target_ref_impl : target_props -> ^(TARGET_NAME EMPTY_TARGET_NAME) target_props
                | target_name_seq target_props?; 
target_name_seq : '//' ID -> ^(TARGET_NAME ID);
target_props : (WS* '/' feature)+ -> ^(FEATURE_SET feature+);
list_of : list_of_impl (WS+ list_of_impl)* -> ^(LIST_OF list_of_impl+);
list_of_impl : path_like_seq
             | target_ref
             | '[' WS* target_decl_or_rule_call_impl WS* ']' -> target_decl_or_rule_call_impl;
public_tag : '@' WS* -> PUBLIC_TAG;
             
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
