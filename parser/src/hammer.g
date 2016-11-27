grammar hammer;

options{ 
   language = C; 
   output = AST;
   ASTLabelType = pANTLR3_BASE_TREE;
}

tokens{
HAMFILE;
PROJECT_DEF;
NO_PROJECT_DEF;
TARGET_DECL_OR_RULE_CALL;
TARGET_REF;
TARGET_NAME;
EMPTY_TARGET_NAME;
ARGUMENTS;
NAMED_EXPRESSION;
EXPRESSION;
EMPTY_EXPRESSION;
LIST_OF;
PATH_LIKE_SEQ;
REQUIREMENT_SET;
REQUIREMENT;
CONDITION;
FEATURE;
}

@parser::preincludes{
}

hamfile       : WS* project_def target_decl_or_rule_call* -> ^(HAMFILE project_def? target_decl_or_rule_call*);
project_def   : 'project' arguments WS* EXP_END WS* -> ^(PROJECT_DEF 'project' arguments EXP_END)
              | -> NO_PROJECT_DEF;

target_decl_or_rule_call : target_decl_or_rule_call_impl WS* EXP_END WS* -> target_decl_or_rule_call_impl EXP_END;
target_decl_or_rule_call_impl : ID arguments -> ^(TARGET_DECL_OR_RULE_CALL ID arguments);

arguments	: (WS+ ';')=> -> ^(ARGUMENTS)
              	|  argument args_leaf* -> ^(ARGUMENTS argument args_leaf*)
		;
args_leaf : WS+ COLON argument -> COLON argument 
	  ;
non_empty_argument 	: expression
 		        | named_argument
 			;		
named_argument 	: argument_name named_argument_expression -> ^(NAMED_EXPRESSION argument_name named_argument_expression)
		;

named_argument_expression 	: (WS+ ':')=> -> EMPTY_EXPRESSION
				| (WS+ ';')=> -> EMPTY_EXPRESSION
				| WS* expression -> expression
				;		
				
argument	: (WS+ ':')=> -> EMPTY_EXPRESSION
		| (WS+ ';')=> -> EMPTY_EXPRESSION
	      	| WS+ non_empty_argument -> non_empty_argument
	      	;
argument_name : ID WS* '=' -> ID;
expression    : requirement_set
              | list_of ;
feature : '<' ID '>' feature_value -> ^(FEATURE ID feature_value);
feature_value : path_like_seq
              | '(' target_ref ')' -> target_ref;
requirement_set   : requirement (WS+ requirement)* -> ^(REQUIREMENT_SET requirement+);
requirement : public_tag? requirement_impl -> ^(REQUIREMENT public_tag? requirement_impl);
requirement_impl : (conditional_requirement)=> conditional_requirement
		 | feature;
conditional_requirement : condition feature -> ^(CONDITION condition) feature;
condition : feature (',' feature)* COLON -> feature* COLON;
// colon must stay here because if it is not syntactic predicate will not work
path_like_seq : SLASH path_like_seq_impl -> ^(PATH_LIKE_SEQ SLASH path_like_seq_impl)
	      |	path_like_seq_impl -> ^(PATH_LIKE_SEQ path_like_seq_impl);
path_like_seq_impl : ID ('/' ID)* '/'? -> ID+;
target_ref : path_like_seq target_ref_impl -> ^(TARGET_REF path_like_seq target_ref_impl)
           | public_tag path_like_seq target_ref_impl? -> ^(TARGET_REF public_tag path_like_seq target_ref_impl?);
target_ref_impl : target_requirements -> ^(TARGET_NAME EMPTY_TARGET_NAME) target_requirements
                | target_name_seq target_requirements?; 
target_name_seq : '//' ID -> ^(TARGET_NAME ID);
target_requirements : (WS* '/' requirement)+ -> ^(REQUIREMENT_SET requirement+);
list_of : list_of_impl (WS+ list_of_impl)* -> ^(LIST_OF list_of_impl+);
list_of_impl : path_like_seq
             | target_ref
             | '[' WS* target_decl_or_rule_call_impl WS* ']' -> target_decl_or_rule_call_impl;
public_tag : PUBLIC_TAG WS* -> PUBLIC_TAG;
             
SLASH : '/';
PUBLIC_TAG : '@';
ID : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_'| '*')+  
	      | STRING ;//{ LEXSTATE->type = _type; {pANTLR3_COMMON_TOKEN t = LEXER->emit(LEXER); ++t->start, --t->stop; t->type = _type;} };
COLON : ':';
EXP_END : ';';

fragment 
STRING : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ); //{ $channel = HIDDEN; };
