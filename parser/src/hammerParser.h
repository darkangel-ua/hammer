/** \file
 *  This C header file was generated by $ANTLR version 3.1.1
 *
 *     -  From the grammar source file : hammer.g
 *     -                            On : 2017-10-16 19:42:57
 *     -                for the parser : hammerParserParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The parser hammerParser has the callable functions (rules) shown below,
 * which will invoke the code for the associated rule in the source grammar
 * assuming that the input stream is pointing to a token/text stream that could begin
 * this rule.
 * 
 * For instance if you call the first (topmost) rule in a parser grammar, you will
 * get the results of a full parse, but calling a rule half way through the grammar will
 * allow you to pass part of a full token stream to the parser, such as for syntax checking
 * in editors and so on.
 *
 * The parser entry points are called indirectly (by function pointer to function) via
 * a parser context typedef phammerParser, which is returned from a call to hammerParserNew().
 *
 * The methods in phammerParser are  as follows:
 *
 *  - hammerParser_hamfile_return      phammerParser->hamfile(phammerParser)
 *  - hammerParser_rule_return      phammerParser->rule(phammerParser)
 *  - hammerParser_rule_prefix_return      phammerParser->rule_prefix(phammerParser)
 *  - hammerParser_rule_invocation_return      phammerParser->rule_invocation(phammerParser)
 *  - hammerParser_rule_impl_return      phammerParser->rule_impl(phammerParser)
 *  - hammerParser_arguments_return      phammerParser->arguments(phammerParser)
 *  - hammerParser_rest_of_arguments_return      phammerParser->rest_of_arguments(phammerParser)
 *  - hammerParser_unnamed_argument_return      phammerParser->unnamed_argument(phammerParser)
 *  - hammerParser_named_argument_return      phammerParser->named_argument(phammerParser)
 *  - hammerParser_named_argument_body_return      phammerParser->named_argument_body(phammerParser)
 *  - hammerParser_argument_return      phammerParser->argument(phammerParser)
 *  - hammerParser_empty_argument_return      phammerParser->empty_argument(phammerParser)
 *  - hammerParser_list_return      phammerParser->list(phammerParser)
 *  - hammerParser_expression_return      phammerParser->expression(phammerParser)
 *  - hammerParser_expressions_a_return      phammerParser->expressions_a(phammerParser)
 *  - hammerParser_expressions_b_return      phammerParser->expressions_b(phammerParser)
 *  - hammerParser_structure_return      phammerParser->structure(phammerParser)
 *  - hammerParser_fields_return      phammerParser->fields(phammerParser)
 *  - hammerParser_field_return      phammerParser->field(phammerParser)
 *  - hammerParser_feature_return      phammerParser->feature(phammerParser)
 *  - hammerParser_feature_value_return      phammerParser->feature_value(phammerParser)
 *  - hammerParser_feature_value_target_return      phammerParser->feature_value_target(phammerParser)
 *  - hammerParser_condition_return      phammerParser->condition(phammerParser)
 *  - hammerParser_condition_condition_return      phammerParser->condition_condition(phammerParser)
 *  - hammerParser_logical_or_return      phammerParser->logical_or(phammerParser)
 *  - hammerParser_logical_and_return      phammerParser->logical_and(phammerParser)
 *  - hammerParser_condition_result_return      phammerParser->condition_result(phammerParser)
 *  - hammerParser_condition_result_elem_return      phammerParser->condition_result_elem(phammerParser)
 *  - hammerParser_path_return      phammerParser->path(phammerParser)
 *  - hammerParser_path_non_uri_return      phammerParser->path_non_uri(phammerParser)
 *  - hammerParser_path_uri_return      phammerParser->path_uri(phammerParser)
 *  - hammerParser_path_root_return      phammerParser->path_root(phammerParser)
 *  - hammerParser_path_rest_return      phammerParser->path_rest(phammerParser)
 *  - hammerParser_path_element_return      phammerParser->path_element(phammerParser)
 *  - hammerParser_wildcard_return      phammerParser->wildcard(phammerParser)
 *  - hammerParser_wildcard_a_return      phammerParser->wildcard_a(phammerParser)
 *  - hammerParser_wildcard_s_return      phammerParser->wildcard_s(phammerParser)
 *  - hammerParser_target_ref_return      phammerParser->target_ref(phammerParser)
 *  - hammerParser_target_ref_impl_return      phammerParser->target_ref_impl(phammerParser)
 *  - hammerParser_target_ref_root_path_return      phammerParser->target_ref_root_path(phammerParser)
 *  - hammerParser_target_ref_spec_return      phammerParser->target_ref_spec(phammerParser)
 *  - hammerParser_target_ref_build_request_return      phammerParser->target_ref_build_request(phammerParser)
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
#ifndef	_hammerParser_H
#define _hammerParser_H
/* =============================================================================
 * Standard antlr3 C runtime definitions
 */
#include    <antlr3.h>

/* End of standard antlr 3 runtime definitions
 * =============================================================================
 */
 
#ifdef __cplusplus
extern "C" {
#endif

// Forward declare the context typedef so that we can use it before it is
// properly defined. Delegators and delegates (from import statements) are
// interdependent and their context structures contain pointers to each other
// C only allows such things to be declared if you pre-declare the typedef.
//
typedef struct hammerParser_Ctx_struct hammerParser, * phammerParser;



#ifdef	ANTLR3_WINDOWS
// Disable: Unreferenced parameter,							- Rules with parameters that are not used
//          constant conditional,							- ANTLR realizes that a prediction is always true (synpred usually)
//          initialized but unused variable					- tree rewrite variables declared but not needed
//          Unreferenced local variable						- lexer rule declares but does not always use _type
//          potentially unitialized variable used			- retval always returned from a rule 
//			unreferenced local function has been removed	- susually getTokenNames or freeScope, they can go without warnigns
//
// These are only really displayed at warning level /W4 but that is the code ideal I am aiming at
// and the codegen must generate some of these warnings by necessity, apart from 4100, which is
// usually generated when a parser rule is given a parameter that it does not use. Mostly though
// this is a matter of orthogonality hence I disable that one.
//
#pragma warning( disable : 4100 )
#pragma warning( disable : 4101 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4189 )
#pragma warning( disable : 4505 )
#endif

/* ========================
 * BACKTRACKING IS ENABLED
 * ========================
 */
typedef struct hammerParser_hamfile_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_hamfile_return;

typedef struct hammerParser_rule_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_rule_return;

typedef struct hammerParser_rule_prefix_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_rule_prefix_return;

typedef struct hammerParser_rule_invocation_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_rule_invocation_return;

typedef struct hammerParser_rule_impl_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_rule_impl_return;

typedef struct hammerParser_arguments_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_arguments_return;

typedef struct hammerParser_rest_of_arguments_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_rest_of_arguments_return;

typedef struct hammerParser_unnamed_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_unnamed_argument_return;

typedef struct hammerParser_named_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_named_argument_return;

typedef struct hammerParser_named_argument_body_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_named_argument_body_return;

typedef struct hammerParser_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_argument_return;

typedef struct hammerParser_empty_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_empty_argument_return;

typedef struct hammerParser_list_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_list_return;

typedef struct hammerParser_expression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_expression_return;

typedef struct hammerParser_expressions_a_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_expressions_a_return;

typedef struct hammerParser_expressions_b_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_expressions_b_return;

typedef struct hammerParser_structure_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_structure_return;

typedef struct hammerParser_fields_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_fields_return;

typedef struct hammerParser_field_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_field_return;

typedef struct hammerParser_feature_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_feature_return;

typedef struct hammerParser_feature_value_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_feature_value_return;

typedef struct hammerParser_feature_value_target_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_feature_value_target_return;

typedef struct hammerParser_condition_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_condition_return;

typedef struct hammerParser_condition_condition_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_condition_condition_return;

typedef struct hammerParser_logical_or_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_logical_or_return;

typedef struct hammerParser_logical_and_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_logical_and_return;

typedef struct hammerParser_condition_result_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_condition_result_return;

typedef struct hammerParser_condition_result_elem_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_condition_result_elem_return;

typedef struct hammerParser_path_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_return;

typedef struct hammerParser_path_non_uri_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_non_uri_return;

typedef struct hammerParser_path_uri_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_uri_return;

typedef struct hammerParser_path_root_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_root_return;

typedef struct hammerParser_path_rest_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_rest_return;

typedef struct hammerParser_path_element_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_path_element_return;

typedef struct hammerParser_wildcard_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_wildcard_return;

typedef struct hammerParser_wildcard_a_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_wildcard_a_return;

typedef struct hammerParser_wildcard_s_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_wildcard_s_return;

typedef struct hammerParser_target_ref_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_target_ref_return;

typedef struct hammerParser_target_ref_impl_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_target_ref_impl_return;

typedef struct hammerParser_target_ref_root_path_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_target_ref_root_path_return;

typedef struct hammerParser_target_ref_spec_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_target_ref_spec_return;

typedef struct hammerParser_target_ref_build_request_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammerParser_target_ref_build_request_return;




/** Context tracking structure for hammerParser
 */
struct hammerParser_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_PARSER   pParser;


     hammerParser_hamfile_return (*hamfile)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_rule_return (*rule)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_rule_prefix_return (*rule_prefix)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_rule_invocation_return (*rule_invocation)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_rule_impl_return (*rule_impl)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_arguments_return (*arguments)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_rest_of_arguments_return (*rest_of_arguments)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_unnamed_argument_return (*unnamed_argument)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_named_argument_return (*named_argument)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_named_argument_body_return (*named_argument_body)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_argument_return (*argument)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_empty_argument_return (*empty_argument)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_list_return (*list)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_expression_return (*expression)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_expressions_a_return (*expressions_a)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_expressions_b_return (*expressions_b)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_structure_return (*structure)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_fields_return (*fields)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_field_return (*field)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_feature_return (*feature)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_feature_value_return (*feature_value)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_feature_value_target_return (*feature_value_target)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_condition_return (*condition)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_condition_condition_return (*condition_condition)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_logical_or_return (*logical_or)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_logical_and_return (*logical_and)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_condition_result_return (*condition_result)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_condition_result_elem_return (*condition_result_elem)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_return (*path)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_non_uri_return (*path_non_uri)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_uri_return (*path_uri)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_root_return (*path_root)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_rest_return (*path_rest)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_path_element_return (*path_element)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_wildcard_return (*wildcard)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_wildcard_a_return (*wildcard_a)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_wildcard_s_return (*wildcard_s)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_target_ref_return (*target_ref)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_target_ref_impl_return (*target_ref_impl)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_target_ref_root_path_return (*target_ref_root_path)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_target_ref_spec_return (*target_ref_spec)	(struct hammerParser_Ctx_struct * ctx);
     hammerParser_target_ref_build_request_return (*target_ref_build_request)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred1_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred2_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred4_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred5_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred6_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred7_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred8_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred9_hammer)	(struct hammerParser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred10_hammer)	(struct hammerParser_Ctx_struct * ctx);
    // Delegated rules
    const char * (*getGrammarFileName)();
    void	    (*free)   (struct hammerParser_Ctx_struct * ctx);
    /* @headerFile.members() */
    pANTLR3_BASE_TREE_ADAPTOR	adaptor;
    pANTLR3_VECTOR_FACTORY		vectors;
    /* End @headerFile.members() */
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API phammerParser hammerParserNew         (pANTLR3_COMMON_TOKEN_STREAM instream);
ANTLR3_API phammerParser hammerParserNewSSD      (pANTLR3_COMMON_TOKEN_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

/** Symbolic definitions of all the tokens that the parser will work with.
 * \{
 *
 * Antlr will define EOF, but we can't use that as it it is too common in
 * in C header files and that would be confusing. There is no way to filter this out at the moment
 * so we just undef it here for now. That isn't the value we get back from C recognizers
 * anyway. We are looking for ANTLR3_TOKEN_EOF.
 */
#ifdef	EOF
#undef	EOF
#endif
#ifdef	Tokens
#undef	Tokens
#endif 
#define T__50      50
#define Hamfile      4
#define Explicit      7
#define Feature      14
#define Wildcard      21
#define Slash      26
#define List      13
#define NamedArgument      10
#define STRING_1      32
#define Arguments      8
#define Local      6
#define EmptyArgument      9
#define Asterix      30
#define Rule      5
#define Id      25
#define StructureField      12
#define RuleInvocation      16
#define LogicalOr      23
#define COMMENT      35
#define T__37      37
#define T__38      38
#define T__39      39
#define LogicalAnd      22
#define STRING_ID      33
#define T__36      36
#define Structure      11
#define WS      24
#define DoubleSlash      27
#define EOF      -1
#define Condition      15
#define Path      17
#define PublicTag      28
#define TargetRefSpec      19
#define STRING_ID1      34
#define T__48      48
#define T__49      49
#define QuestionMark      29
#define TargetRef      18
#define T__44      44
#define T__45      45
#define STRING      31
#define T__46      46
#define T__47      47
#define T__40      40
#define T__41      41
#define T__42      42
#define T__43      43
#define TargetRefBuildRequest      20
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for hammerParser
 * =============================================================================
 */
/** \} */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
