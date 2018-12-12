/** \file
 *  This C header file was generated by $ANTLR version 3.1.1
 *
 *     -  From the grammar source file : hammer_v2.g
 *     -                            On : 2018-12-08 18:07:29
 *     -                for the parser : hammer_v2ParserParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The parser hammer_v2Parser has the callable functions (rules) shown below,
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
 * a parser context typedef phammer_v2Parser, which is returned from a call to hammer_v2ParserNew().
 *
 * The methods in phammer_v2Parser are  as follows:
 *
 *  - hammer_v2Parser_hamfile_return      phammer_v2Parser->hamfile(phammer_v2Parser)
 *  - hammer_v2Parser_rule_return      phammer_v2Parser->rule(phammer_v2Parser)
 *  - hammer_v2Parser_rule_prefix_return      phammer_v2Parser->rule_prefix(phammer_v2Parser)
 *  - hammer_v2Parser_rule_invocation_return      phammer_v2Parser->rule_invocation(phammer_v2Parser)
 *  - hammer_v2Parser_rule_impl_return      phammer_v2Parser->rule_impl(phammer_v2Parser)
 *  - hammer_v2Parser_arguments_return      phammer_v2Parser->arguments(phammer_v2Parser)
 *  - hammer_v2Parser_rest_of_arguments_return      phammer_v2Parser->rest_of_arguments(phammer_v2Parser)
 *  - hammer_v2Parser_unnamed_argument_return      phammer_v2Parser->unnamed_argument(phammer_v2Parser)
 *  - hammer_v2Parser_named_argument_return      phammer_v2Parser->named_argument(phammer_v2Parser)
 *  - hammer_v2Parser_named_argument_body_return      phammer_v2Parser->named_argument_body(phammer_v2Parser)
 *  - hammer_v2Parser_argument_return      phammer_v2Parser->argument(phammer_v2Parser)
 *  - hammer_v2Parser_empty_argument_return      phammer_v2Parser->empty_argument(phammer_v2Parser)
 *  - hammer_v2Parser_list_return      phammer_v2Parser->list(phammer_v2Parser)
 *  - hammer_v2Parser_expression_return      phammer_v2Parser->expression(phammer_v2Parser)
 *  - hammer_v2Parser_expressions_a_return      phammer_v2Parser->expressions_a(phammer_v2Parser)
 *  - hammer_v2Parser_expressions_b_return      phammer_v2Parser->expressions_b(phammer_v2Parser)
 *  - hammer_v2Parser_structure_return      phammer_v2Parser->structure(phammer_v2Parser)
 *  - hammer_v2Parser_structure_body_return      phammer_v2Parser->structure_body(phammer_v2Parser)
 *  - hammer_v2Parser_fields_return      phammer_v2Parser->fields(phammer_v2Parser)
 *  - hammer_v2Parser_rest_of_fields_return      phammer_v2Parser->rest_of_fields(phammer_v2Parser)
 *  - hammer_v2Parser_field_return      phammer_v2Parser->field(phammer_v2Parser)
 *  - hammer_v2Parser_empty_field_return      phammer_v2Parser->empty_field(phammer_v2Parser)
 *  - hammer_v2Parser_unnamed_field_return      phammer_v2Parser->unnamed_field(phammer_v2Parser)
 *  - hammer_v2Parser_named_field_return      phammer_v2Parser->named_field(phammer_v2Parser)
 *  - hammer_v2Parser_named_field_body_return      phammer_v2Parser->named_field_body(phammer_v2Parser)
 *  - hammer_v2Parser_feature_return      phammer_v2Parser->feature(phammer_v2Parser)
 *  - hammer_v2Parser_feature_value_return      phammer_v2Parser->feature_value(phammer_v2Parser)
 *  - hammer_v2Parser_feature_value_target_return      phammer_v2Parser->feature_value_target(phammer_v2Parser)
 *  - hammer_v2Parser_condition_return      phammer_v2Parser->condition(phammer_v2Parser)
 *  - hammer_v2Parser_condition_condition_return      phammer_v2Parser->condition_condition(phammer_v2Parser)
 *  - hammer_v2Parser_logical_or_return      phammer_v2Parser->logical_or(phammer_v2Parser)
 *  - hammer_v2Parser_logical_and_return      phammer_v2Parser->logical_and(phammer_v2Parser)
 *  - hammer_v2Parser_condition_result_return      phammer_v2Parser->condition_result(phammer_v2Parser)
 *  - hammer_v2Parser_condition_result_elem_return      phammer_v2Parser->condition_result_elem(phammer_v2Parser)
 *  - hammer_v2Parser_path_return      phammer_v2Parser->path(phammer_v2Parser)
 *  - hammer_v2Parser_path_uri_return      phammer_v2Parser->path_uri(phammer_v2Parser)
 *  - hammer_v2Parser_path_without_root_return      phammer_v2Parser->path_without_root(phammer_v2Parser)
 *  - hammer_v2Parser_path_with_root_return      phammer_v2Parser->path_with_root(phammer_v2Parser)
 *  - hammer_v2Parser_path_root_name_return      phammer_v2Parser->path_root_name(phammer_v2Parser)
 *  - hammer_v2Parser_path_rest_return      phammer_v2Parser->path_rest(phammer_v2Parser)
 *  - hammer_v2Parser_path_element_return      phammer_v2Parser->path_element(phammer_v2Parser)
 *  - hammer_v2Parser_wildcard_return      phammer_v2Parser->wildcard(phammer_v2Parser)
 *  - hammer_v2Parser_wildcard_a_return      phammer_v2Parser->wildcard_a(phammer_v2Parser)
 *  - hammer_v2Parser_wildcard_s_return      phammer_v2Parser->wildcard_s(phammer_v2Parser)
 *  - hammer_v2Parser_target_ref_return      phammer_v2Parser->target_ref(phammer_v2Parser)
 *  - hammer_v2Parser_target_ref_impl_return      phammer_v2Parser->target_ref_impl(phammer_v2Parser)
 *  - hammer_v2Parser_target_ref_root_path_return      phammer_v2Parser->target_ref_root_path(phammer_v2Parser)
 *  - hammer_v2Parser_target_ref_spec_return      phammer_v2Parser->target_ref_spec(phammer_v2Parser)
 *  - hammer_v2Parser_target_ref_build_request_return      phammer_v2Parser->target_ref_build_request(phammer_v2Parser)
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
 * 
 * 
 * 
 * 
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
#ifndef	_hammer_v2Parser_H
#define _hammer_v2Parser_H
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
typedef struct hammer_v2Parser_Ctx_struct hammer_v2Parser, * phammer_v2Parser;



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
typedef struct hammer_v2Parser_hamfile_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_hamfile_return;

typedef struct hammer_v2Parser_rule_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rule_return;

typedef struct hammer_v2Parser_rule_prefix_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rule_prefix_return;

typedef struct hammer_v2Parser_rule_invocation_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rule_invocation_return;

typedef struct hammer_v2Parser_rule_impl_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rule_impl_return;

typedef struct hammer_v2Parser_arguments_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_arguments_return;

typedef struct hammer_v2Parser_rest_of_arguments_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rest_of_arguments_return;

typedef struct hammer_v2Parser_unnamed_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_unnamed_argument_return;

typedef struct hammer_v2Parser_named_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_named_argument_return;

typedef struct hammer_v2Parser_named_argument_body_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_named_argument_body_return;

typedef struct hammer_v2Parser_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_argument_return;

typedef struct hammer_v2Parser_empty_argument_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_empty_argument_return;

typedef struct hammer_v2Parser_list_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_list_return;

typedef struct hammer_v2Parser_expression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_expression_return;

typedef struct hammer_v2Parser_expressions_a_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_expressions_a_return;

typedef struct hammer_v2Parser_expressions_b_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_expressions_b_return;

typedef struct hammer_v2Parser_structure_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_structure_return;

typedef struct hammer_v2Parser_structure_body_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_structure_body_return;

typedef struct hammer_v2Parser_fields_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_fields_return;

typedef struct hammer_v2Parser_rest_of_fields_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_rest_of_fields_return;

typedef struct hammer_v2Parser_field_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_field_return;

typedef struct hammer_v2Parser_empty_field_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_empty_field_return;

typedef struct hammer_v2Parser_unnamed_field_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_unnamed_field_return;

typedef struct hammer_v2Parser_named_field_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_named_field_return;

typedef struct hammer_v2Parser_named_field_body_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_named_field_body_return;

typedef struct hammer_v2Parser_feature_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_feature_return;

typedef struct hammer_v2Parser_feature_value_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_feature_value_return;

typedef struct hammer_v2Parser_feature_value_target_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_feature_value_target_return;

typedef struct hammer_v2Parser_condition_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_condition_return;

typedef struct hammer_v2Parser_condition_condition_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_condition_condition_return;

typedef struct hammer_v2Parser_logical_or_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_logical_or_return;

typedef struct hammer_v2Parser_logical_and_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_logical_and_return;

typedef struct hammer_v2Parser_condition_result_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_condition_result_return;

typedef struct hammer_v2Parser_condition_result_elem_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_condition_result_elem_return;

typedef struct hammer_v2Parser_path_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_return;

typedef struct hammer_v2Parser_path_uri_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_uri_return;

typedef struct hammer_v2Parser_path_without_root_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_without_root_return;

typedef struct hammer_v2Parser_path_with_root_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_with_root_return;

typedef struct hammer_v2Parser_path_root_name_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_root_name_return;

typedef struct hammer_v2Parser_path_rest_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_rest_return;

typedef struct hammer_v2Parser_path_element_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_path_element_return;

typedef struct hammer_v2Parser_wildcard_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_wildcard_return;

typedef struct hammer_v2Parser_wildcard_a_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_wildcard_a_return;

typedef struct hammer_v2Parser_wildcard_s_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_wildcard_s_return;

typedef struct hammer_v2Parser_target_ref_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_target_ref_return;

typedef struct hammer_v2Parser_target_ref_impl_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_target_ref_impl_return;

typedef struct hammer_v2Parser_target_ref_root_path_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_target_ref_root_path_return;

typedef struct hammer_v2Parser_target_ref_spec_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_target_ref_spec_return;

typedef struct hammer_v2Parser_target_ref_build_request_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;
   
}
    hammer_v2Parser_target_ref_build_request_return;




/** Context tracking structure for hammer_v2Parser
 */
struct hammer_v2Parser_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_PARSER   pParser;


     hammer_v2Parser_hamfile_return (*hamfile)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rule_return (*rule)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rule_prefix_return (*rule_prefix)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rule_invocation_return (*rule_invocation)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rule_impl_return (*rule_impl)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_arguments_return (*arguments)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rest_of_arguments_return (*rest_of_arguments)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_unnamed_argument_return (*unnamed_argument)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_named_argument_return (*named_argument)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_named_argument_body_return (*named_argument_body)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_argument_return (*argument)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_empty_argument_return (*empty_argument)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_list_return (*list)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_expression_return (*expression)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_expressions_a_return (*expressions_a)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_expressions_b_return (*expressions_b)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_structure_return (*structure)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_structure_body_return (*structure_body)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_fields_return (*fields)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_rest_of_fields_return (*rest_of_fields)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_field_return (*field)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_empty_field_return (*empty_field)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_unnamed_field_return (*unnamed_field)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_named_field_return (*named_field)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_named_field_body_return (*named_field_body)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_feature_return (*feature)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_feature_value_return (*feature_value)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_feature_value_target_return (*feature_value_target)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_condition_return (*condition)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_condition_condition_return (*condition_condition)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_logical_or_return (*logical_or)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_logical_and_return (*logical_and)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_condition_result_return (*condition_result)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_condition_result_elem_return (*condition_result_elem)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_return (*path)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_uri_return (*path_uri)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_without_root_return (*path_without_root)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_with_root_return (*path_with_root)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_root_name_return (*path_root_name)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_rest_return (*path_rest)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_path_element_return (*path_element)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_wildcard_return (*wildcard)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_wildcard_a_return (*wildcard_a)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_wildcard_s_return (*wildcard_s)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_target_ref_return (*target_ref)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_target_ref_impl_return (*target_ref_impl)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_target_ref_root_path_return (*target_ref_root_path)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_target_ref_spec_return (*target_ref_spec)	(struct hammer_v2Parser_Ctx_struct * ctx);
     hammer_v2Parser_target_ref_build_request_return (*target_ref_build_request)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred1_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred2_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred4_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred5_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred6_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred8_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred9_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred10_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred11_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred12_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred13_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred14_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred15_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
     ANTLR3_BOOLEAN (*synpred16_hammer_v2)	(struct hammer_v2Parser_Ctx_struct * ctx);
    // Delegated rules
    const char * (*getGrammarFileName)();
    void	    (*free)   (struct hammer_v2Parser_Ctx_struct * ctx);
    /* @headerFile.members() */
    pANTLR3_BASE_TREE_ADAPTOR	adaptor;
    pANTLR3_VECTOR_FACTORY		vectors;
    /* End @headerFile.members() */
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API phammer_v2Parser hammer_v2ParserNew         (pANTLR3_COMMON_TOKEN_STREAM instream);
ANTLR3_API phammer_v2Parser hammer_v2ParserNewSSD      (pANTLR3_COMMON_TOKEN_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

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
#define EmptyField      12
#define Explicit      7
#define PathTrailingSlash      20
#define Feature      15
#define Wildcard      24
#define Slash      30
#define List      14
#define T__51      51
#define NamedArgument      10
#define T__52      52
#define T__53      53
#define STRING_1      36
#define Arguments      8
#define Local      6
#define EmptyArgument      9
#define Asterix      34
#define Rule      5
#define Id      28
#define RuleInvocation      17
#define LogicalOr      26
#define COMMENT      39
#define NamedField      13
#define PathRootName      19
#define LogicalAnd      25
#define STRING_ID      37
#define Structure      11
#define WS      27
#define DoubleSlash      32
#define EOF      -1
#define Condition      16
#define Path      18
#define PublicTag      29
#define TargetRefSpec      22
#define PathUri      31
#define STRING_ID1      38
#define T__48      48
#define T__49      49
#define QuestionMark      33
#define TargetRef      21
#define T__44      44
#define T__45      45
#define STRING      35
#define T__46      46
#define T__47      47
#define T__40      40
#define T__41      41
#define T__42      42
#define T__43      43
#define TargetRefBuildRequest      23
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for hammer_v2Parser
 * =============================================================================
 */
/** \} */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
