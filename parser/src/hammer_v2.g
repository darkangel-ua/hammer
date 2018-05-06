grammar hammer_v2;

options{ 
   language = C; 
   output = AST;
   ASTLabelType = pANTLR3_BASE_TREE;
}

tokens{
Hamfile;
Rule;
Local;
Explicit;
Arguments;
EmptyArgument;
NamedArgument;
Structure;
StructureField;
List;
Feature;
Condition;
RuleInvocation;
Path;
PathRootName;
PathTrailingSlash;
TargetRef;
TargetRefSpec;
TargetRefBuildRequest;
Wildcard;
LogicalAnd;
LogicalOr;
}

hamfile 
	: WS* rule* -> ^(Hamfile rule*)
	;
rule 	
	: (rule_prefix WS+)? rule_impl ';' WS* -> ^(Rule rule_prefix? rule_impl)
	;
rule_prefix 
	: Local
	| Explicit
	;
rule_invocation 
	: '[' WS* rule_impl ']' -> ^(RuleInvocation rule_impl)
	;	
rule_impl 
	: Id WS+ arguments -> Id arguments
	;
arguments 
	: (';')=> -> Arguments
	| argument rest_of_arguments* -> ^(Arguments argument rest_of_arguments*)
	;
rest_of_arguments 
	: ':' WS* argument -> argument
	;
unnamed_argument
	: (list)=> list WS* -> list
	| expression WS* -> expression
	;	
named_argument
	: Id WS* '=' WS* named_argument_body -> ^(NamedArgument Id named_argument_body)
	;	
named_argument_body
	: empty_argument
	| unnamed_argument
	;
argument 
	: empty_argument
	| named_argument
	| unnamed_argument
	;
empty_argument
	: (':' | ';') => -> EmptyArgument[LT(1), LT(1)->getText(LT(1))->chars]
	;
list 	
	: expression (WS+ expression)+ -> ^(List expression+)
	;
expression
	: (target_ref)=> target_ref
	| t='@' WS* expressions_a -> ^(PublicTag[$t, $t->getText($t)->chars] expressions_a)
	| expressions_a
	| expressions_b
	| (path)=> path
	| Id
	;
expressions_a 
	: condition
	| feature
	; 
expressions_b
	: structure
	| rule_invocation
	;	
structure 
	: '{' WS* fields '}' -> ^(Structure fields)
	;
fields 	
	: field (':' WS* field)* -> field field*
 	;		
field 	
	: Id WS* '=' WS* unnamed_argument -> ^(StructureField Id unnamed_argument)
 	;
feature 
	: '<' WS* Id WS* '>' WS* feature_value -> ^(Feature Id feature_value)
	;
feature_value
	: path
	| Id
	| '(' WS* feature_value_target WS* ')' -> feature_value_target
	;	
feature_value_target 
	: (target_ref) => target_ref
	| (path) => path -> ^(TargetRef path)
	| Id -> ^(TargetRef Id)
	;	
condition
	: '(' WS* condition_condition condition_result WS* ')' -> ^(Condition condition_condition condition_result)
	;
condition_condition
	: logical_or WS* '->' WS* -> logical_or
	;
logical_or
options { backtrack = true; }
	: logical_and WS* '||' WS* logical_and -> ^(LogicalOr logical_and logical_and)
	| logical_and
	;	
logical_and 
options { backtrack = true; }
	: feature WS* '&&' WS* logical_and -> ^(LogicalAnd feature logical_and)
	| feature
	;
condition_result 
	: (condition_result_elem WS+ condition_result_elem)=> condition_result_elem (WS+ condition_result_elem)+ -> ^(List condition_result_elem+)
	| condition_result_elem
	;
condition_result_elem
	: PublicTag WS* feature -> ^(PublicTag feature)
	| feature
	;	
path
	: path_without_root
	| path_uri
	;
path_uri
	: 'file://' path_with_root -> path_with_root
	;
path_without_root 
	: path_element path_rest+ -> ^(Path path_element path_rest+) 
	;	
path_with_root
	: path_root_name path_element path_rest* -> ^(Path path_root_name path_element path_rest*)
	;
path_root_name 
	: r=Slash -> PathRootName[$r, $r->getText($r)->chars]
	| r=Id ':' '/' -> PathRootName[$r, $r->getText($r)->chars]
	;
path_rest 
	: ('/' path_element) => '/' path_element -> path_element
	| s='/' -> PathTrailingSlash[$s, $s->getText($s)->chars]
	;
path_element 
	: wildcard
	| Id
	;
wildcard 
	: wildcard_s wildcard_a? -> ^(Wildcard wildcard_s wildcard_a?)
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
target_ref
	: PublicTag WS* target_ref_impl -> ^(PublicTag target_ref_impl)
	| ('/') => target_ref_root_path
	| path_without_root target_ref_spec -> ^(TargetRef path_without_root target_ref_spec)
	| Id target_ref_spec -> ^(TargetRef Id target_ref_spec)
	;	
target_ref_impl 
	: ('/') => target_ref_root_path
	| path_without_root target_ref_spec? -> ^(TargetRef path_without_root target_ref_spec?)
	| Id target_ref_spec? -> ^(TargetRef Id target_ref_spec?)
	;
target_ref_root_path
	: path_with_root target_ref_spec? -> ^(TargetRef path_with_root target_ref_spec?)
	;
target_ref_spec
	: '//' Id target_ref_build_request? -> ^(TargetRefSpec Id target_ref_build_request?)
	| target_ref_build_request -> ^(TargetRefSpec target_ref_build_request)
	;	
target_ref_build_request
	: ('/' feature)+ -> ^(TargetRefBuildRequest feature+)
	;	

PathUri : 'file://';
Slash : '/';
DoubleSlash : '//';
PublicTag : '@';
Local : 'local' ;
Explicit : 'explicit' ;
QuestionMark : '?';
Asterix : '*';

Id : ('a'..'z' | 'A'..'Z' | '0'..'9' | '.' | '-' | '_' | '+')+
   | STRING | STRING_1
   ;
fragment 
STRING : '"' STRING_ID '"';
fragment
STRING_ID : ('\\"' | ~('"' | '\n' | '\r'))* ;
fragment
STRING_1: '\'' STRING_ID1 '\'';
fragment
STRING_ID1 : ('\\\'' | ~('\'' | '\n' | '\r'))* ;

COMMENT : '#' (~('\n' | '\r'))* ('\n' | '\r')?  { $channel = HIDDEN; };

WS : (' ' |'\n' |'\r' | '\t'); //{ $channel = HIDDEN; };
