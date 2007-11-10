/** \file
 *  This C header file was generated by $ANTLR version 3.0.1
 *
 *     -  From the grammar source file : hammer_walker.g
 *     -                            On : 2007-11-10 13:24:48
 *     -           for the tree parser : hammer_walkerTreeParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 * View this file with tabs set to 8 (:set ts=8 in gvim) and indent at 4 (:set sw=4 in gvim)
 *
 * The tree parser hammer_walkerhas the callable functions (rules) shown below,
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
 * The entry points for hammer_walker are  as follows:
 *
 *  - void      phammer_walker->project(phammer_walker)
 *  - void      phammer_walker->meta_target(phammer_walker)
 *  - void      phammer_walker->project_def(phammer_walker)
 *  - void      phammer_walker->lib(phammer_walker)
 *  - void      phammer_walker->exe(phammer_walker)
 *  - void      phammer_walker->sources(phammer_walker)
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

	#include "../hammer_walker_impl.h"


#ifdef	WIN32
// Disable: Unreferenced parameter,                - Rules with parameters that are not used
//          constant conditional,                  - ANTLR realizes that a prediction is always true (synpred usually)
//          initialized but unused variable        - tree rewrite vairables declared but not needed
//          Unreferenced local variable            - lexer rulle decalres but does not always use _type
//          potentially unitialized variable used  - retval always returned from a rule 
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
#pragma warning( disable : 4701 )
#endif



/** Context tracking structure for hammer_walker
 */
typedef struct hammer_walker_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_TREE_PARSER	    pTreeParser;

    void (*project)	(struct hammer_walker_Ctx_struct * ctx);
    void (*meta_target)	(struct hammer_walker_Ctx_struct * ctx);
    void (*project_def)	(struct hammer_walker_Ctx_struct * ctx);
    void (*lib)	(struct hammer_walker_Ctx_struct * ctx);
    void (*exe)	(struct hammer_walker_Ctx_struct * ctx);
    void (*sources)	(struct hammer_walker_Ctx_struct * ctx, void* mt);    unsigned char * (*getGrammarFileName)();
    void	    (*free)   (struct hammer_walker_Ctx_struct * ctx);
        
}
    hammer_walker, * phammer_walker;

/* Function protoypes for the treeparser functions that external translation units
 * may wish to call.
 */
ANTLR3_API phammer_walker hammer_walkerNew         (pANTLR3_COMMON_TREE_NODE_STREAM     instream);/** Symbolic definitions of all the tokens that the tree parser will work with.
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
#define EOF      -1
#define WS      13
#define ASSIGN      10
#define LIB      6
#define JAM_STRING      11
#define PROJECT_DEF      5
#define PROJECT      4
#define SOURCES      8
#define EXE      7
#define ID      9
#define JAM_COMMENT      12
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

/* End of token definitions for hammer_walker
 * =============================================================================
 */
/** \} */

#endif
/* END - Note:Keep extra linefeed to satisfy UNIX systems */
