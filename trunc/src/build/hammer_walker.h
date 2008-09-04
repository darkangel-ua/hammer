/** \file
 *  This C header file was generated by $ANTLR version 3.1b1
 *
 *     -  From the grammar source file : hammer_walker.g
 *     -                            On : 2008-09-04 22:35:44
 *     -           for the tree parser : hammer_walkerTreeParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The tree parser hammer_walker has the callable functions (rules) shown below,
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
 * a parser context typedef phammer_walker, which is returned from a call to hammer_walkerNew().
 *
 * The methods in phammer_walker are  as follows:
 *
 *  - void      phammer_walker->project(phammer_walker)
 *  - void      phammer_walker->rules(phammer_walker)
 *  - void*      phammer_walker->rule(phammer_walker)
 *  - void      phammer_walker->args(phammer_walker)
 *  - void      phammer_walker->string_arg(phammer_walker)
 *  - void      phammer_walker->string_list(phammer_walker)
 *  - void      phammer_walker->string_list_id(phammer_walker)
 *  - void      phammer_walker->feature_list(phammer_walker)
 *  - void      phammer_walker->feature_arg(phammer_walker)
 *  - void      phammer_walker->feature(phammer_walker)
 *  - void      phammer_walker->null_arg(phammer_walker)
 *  - void*      phammer_walker->project_requirements(phammer_walker)
 *  - void*      phammer_walker->requirements(phammer_walker)
 *  - void*      phammer_walker->conditional_features(phammer_walker)
 *  - void      phammer_walker->condition(phammer_walker)
 *  - void*      phammer_walker->cfeature(phammer_walker)
 *  - void*      phammer_walker->sources_decl(phammer_walker)
 *  - void*      phammer_walker->source_decl(phammer_walker)
 *  - void*      phammer_walker->target_path(phammer_walker)
 *  - void      phammer_walker->target_name(phammer_walker)
 *  - void      phammer_walker->target_features(phammer_walker)
 *  - void      phammer_walker->sources_decl_rule_invoke(phammer_walker)
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
#ifndef	_hammer_walker_H
#define _hammer_walker_H
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
typedef struct hammer_walker_Ctx_struct hammer_walker, * phammer_walker;



        #include "../hammer_walker_impl.h"


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


/** Context tracking structure for hammer_walker
 */
struct hammer_walker_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_TREE_PARSER	    pTreeParser;


     void (*project)	(struct hammer_walker_Ctx_struct * ctx);
     void (*rules)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*rule)	(struct hammer_walker_Ctx_struct * ctx);
     void (*args)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void (*string_arg)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void (*string_list)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void (*string_list_id)	(struct hammer_walker_Ctx_struct * ctx, void* list);
     void (*feature_list)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void (*feature_arg)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void (*feature)	(struct hammer_walker_Ctx_struct * ctx, void* list);
     void (*null_arg)	(struct hammer_walker_Ctx_struct * ctx, void* args_list);
     void* (*project_requirements)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*requirements)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*conditional_features)	(struct hammer_walker_Ctx_struct * ctx);
     void (*condition)	(struct hammer_walker_Ctx_struct * ctx, void* c);
     void* (*cfeature)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*sources_decl)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*source_decl)	(struct hammer_walker_Ctx_struct * ctx);
     void* (*target_path)	(struct hammer_walker_Ctx_struct * ctx);
     void (*target_name)	(struct hammer_walker_Ctx_struct * ctx, void* sd);
     void (*target_features)	(struct hammer_walker_Ctx_struct * ctx);
     void (*sources_decl_rule_invoke)	(struct hammer_walker_Ctx_struct * ctx, void* sources);
    // Delegated rules
    const char * (*getGrammarFileName)();
    void	    (*free)   (struct hammer_walker_Ctx_struct * ctx);
        
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API phammer_walker hammer_walkerNew         (pANTLR3_COMMON_TREE_NODE_STREAM instream);
ANTLR3_API phammer_walker hammer_walkerNewSSD      (pANTLR3_COMMON_TREE_NODE_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

/** Symbolic definitions of all the tokens that the tree parser will work with.
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
#define REQUIREMENTS_DECL      10
#define STRING_LIST      7
#define T__29      29
#define SOURCE_DECL      15
#define T__28      28
#define TARGET_NAME      17
#define T__27      27
#define TARGET_PATH      16
#define STRING_ID      25
#define SOURCE_DECL_EXPLICIT_TARGET      19
#define CONDITIONAL_FEATURES      11
#define CONDITION      12
#define ID      21
#define TARGET_FEATURES      18
#define EOF      -1
#define NULL_ARG      5
#define FEATURE      9
#define STRING_ARG      6
#define COLON      22
#define T__30      30
#define T__31      31
#define T__32      32
#define WS      20
#define SLASH      23
#define SOURCES_DECL      14
#define PROJECT_REQUIREMENTS      13
#define FEATURE_LIST      8
#define RULE_CALL      4
#define COMMENT      26
#define STRING      24
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for hammer_walker
 * =============================================================================
 */
/** \} */

#endif

#ifdef __cplusplus
}
#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
