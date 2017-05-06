grammar hammer;

options{ 
   language = Java; 
   output = AST;
//   ASTLabelType = pANTLR3_BASE_TREE;
}

tokens{
Hamfile;
Rule;
Local;
NonLocal;
Arguments;
EmptyArgument;
NamedArgument;
Structure;
StructureField;
List;
Feature;
RuleInvocation;
Path;
Target;
Wildcard;
Public;
}

hamfile 
	: WS* rule* -> ^(Hamfile rule*)
	;
rule 	
	: rule_prefix rule_impl ';' WS* -> ^(Rule rule_prefix rule_impl)
	;
rule_prefix 
	: 'local' WS+ -> Local
	| -> NonLocal
	;	
arguments 
	: (';')=> -> Arguments
	| argument rest_of_arguments* -> ^(Arguments argument rest_of_arguments*)
	;
rest_of_arguments 
	: ':' WS* argument -> argument
	;
unnamed_argument
	: (list)=> list
	| expression
	;	
named_argument
	: Id WS* '=' named_argument_body -> ^(NamedArgument Id named_argument_body)
	;	
named_argument_body
	: (':' | ';')=> -> EmptyArgument
	| unnamed_argument
	;
argument 
	: (':' | ';')=> -> EmptyArgument
        | named_argument
        | unnamed_argument
	;
list 	
	: list_e -> ^(List list_e)
	;
list_e
	: (list_b WS* list_a) => list_b WS* list_a list_ee? -> list_b list_a list_ee?
	| (list_b WS+ list_b WS+)=> list_b WS+ list_b WS+ list_ee? -> list_b list_b list_ee?
	| (list_b WS+ list_b list_a)=> list_b WS+ list_b list_a list_ee? -> list_b list_b list_a list_ee?
	| list_b WS+ list_b -> list_b list_b
	;
list_ee 
options { backtrack = true; }
	: list_b WS* list_a list_ee? -> list_b list_a list_ee?
	| list_b WS+ list_b WS+ list_ee? -> list_b list_b list_ee?
	| list_b WS+ list_b list_a list_ee? -> list_b list_b list_a list_ee?
	| list_b WS+ list_b -> list_b list_b
	| list_a list_ee?
	| list_b WS* -> list_b
	;
list_b 	
 	: (target)=> target
 	| (path)=> path
 	| Id
	;
list_a 	
	: structure
	| feature
	| rule_invocation
	;
expression
	: (target)=> target WS* -> target
	| (path)=> path WS* -> path
	| structure
	| some_feature
	| rule_invocation
	| simple_id
	;
simple_id 
	: Id WS* -> Id
	;
structure 
	: '{' WS* fields '}' WS* -> ^(Structure fields)
	;
fields 	
	: field (':' WS* field)* -> field field*
 	;		
field 	
	: Id WS* '=' unnamed_argument -> ^(StructureField Id unnamed_argument)
 	;
some_feature
	: public_feature
	| feature	
	;
public_feature
	: '@' WS* feature -> ^(Public feature)	
	;
feature 
	: feature_impl WS* -> feature_impl
	;	 	
feature_impl
	: '<' WS* Id WS* '>' WS* feature_value -> ^(Feature Id feature_value)
	;
feature_value
	: (path)=>path
	| Id
	;	
rule_invocation 
	: '[' WS* rule_impl ']' WS* -> ^(RuleInvocation rule_impl)
	;	
rule_impl 
	: Id WS+ arguments -> Id arguments
	;
path
	: ('/')=> path_rest+ -> ^(Path Slash path_rest+)
	| path_element path_rest+ -> ^(Path path_element path_rest+)
	;
path_rest 
	: '/' path_element -> path_element
	;
path_element 
	: wildcard
	| Id
	;
wildcard 
	: wildcard_s wildcard_a? -> ^(Wildcard wildcard_s? wildcard_a?)
	| Id wildcard_s wildcard_a? -> ^(Wildcard Id wildcard_s wildcard_a?)
	;
wildcard_a 
	: Id wildcard_s wildcard_a
	| Id
	;	
wildcard_s 
	: '*'
	| '?'+
	;	           
target 	
options { backtrack = true; }
	: '@' WS* target_impl -> ^(Public target_impl)
	| '@' WS* path -> ^(Public ^(Target path))
	| '@' WS* Id -> ^(Public ^(Target Id))
	| target_impl -> target_impl
	;	
target_impl 
	: path '//' target_spec -> ^(Target path target_spec)
	;
target_spec
	: Id target_build_request? -> Id (^(List target_build_request))?
	;	
target_build_request
	: ('/' feature_impl)+ -> feature_impl+
	;	
Slash 	:	 '/';
DoubleSlash :	 '//';
PublicTag : '@';

Id : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+
	      | STRING ;//{ LEXSTATE->type = _type; {pANTLR3_COMMON_TOKEN t = LEXER->emit(LEXER); ++t->start, --t->stop; t->type = _type;} };
fragment 
STRING : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ); //{ $channel = HIDDEN; };
