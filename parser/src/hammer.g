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
Arguments;
EmptyArgument;
NamedArgument;
Structure;
StructureField;
List;
Feature;
ConditionalFeature;
RuleInvocation;
Path;
Target;
Wildcard;
Public;
FeatureLogicalAnd;
FeatureLogicalOr;
}

hamfile 
	: WS* rule* -> ^(Hamfile rule*)
	;
rule 	
	: (Local WS+)? rule_impl ';' WS* -> ^(Rule Local? rule_impl)
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
	: Id WS* '=' WS* named_argument_body -> ^(NamedArgument Id named_argument_body)
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
	| list_a list_ee
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
 	| (conditional_feature_b)=> conditional_feature_b
 	| feature_impl
 	| Id
	;
list_a 	
	: structure
	| rule_invocation
	| conditional_feature_a
	;
expression
	: (target)=> target WS* -> target
	| (path)=> path WS* -> path
	| (conditional_feature)=> conditional_feature WS* -> conditional_feature
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
	: Id WS* '=' WS* unnamed_argument -> ^(StructureField Id unnamed_argument)
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
	: Id
	| '(' WS* feature_value_path WS* ')' -> feature_value_path
	;	
feature_value_path 
	: (target)=> target
	| (path)=> path
	;	
conditional_feature
	: conditional_feature_condition conditional_feature_ab -> ^(ConditionalFeature conditional_feature_condition conditional_feature_ab)
	;
conditional_feature_condition
	: condition_or WS* '->' WS* -> condition_or
	;
condition_or
options { backtrack = true; }
	: condition_and WS* '||' WS* condition_and -> ^(FeatureLogicalOr condition_and condition_and)
	| condition_and
	;	
condition_and 
options { backtrack = true; }
	: feature_impl WS* '&&' WS* condition_and -> ^(FeatureLogicalAnd feature_impl condition_and)
	| feature_impl
	;
conditional_feature_ab 
	: conditional_feature_a
	| conditional_feature_b
	;
conditional_feature_a
	: '(' (WS* feature_impl)+ WS* ')' -> ^(List feature_impl+)
	;
conditional_feature_b
	: feature_impl
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
	: path target_spec -> ^(Target path target_spec)
	;
target_spec
	: '//' Id target_build_request? -> Id (^(List target_build_request))?
	| target_build_request -> ^(List target_build_request)
	;	
target_build_request
	: ('/' feature_impl)+ -> feature_impl+
	;	
Slash : '/';
DoubleSlash : '//';
PublicTag : '@';
Local : 'local' ;

Id : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_')+
   | STRING ;
fragment 
STRING : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' ); //{ $channel = HIDDEN; };
