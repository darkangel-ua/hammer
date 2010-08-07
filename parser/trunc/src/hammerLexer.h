/** \file
 *  This C header file was generated by $ANTLR version 3.1.1
 *
 *     -  From the grammar source file : hammer.g
 *     -                            On : 2010-08-06 17:55:43
 *     -                 for the lexer : hammerLexerLexer *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The lexer hammerLexer has the callable functions (rules) shown below,
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
 * a parser context typedef phammerLexer, which is returned from a call to hammerLexerNew().
 *
 * As this is a generated lexer, it is unlikely you will call it 'manually'. However
 * the methods are provided anyway.
 * * The methods in phammerLexer are  as follows:
 *
 *  -  void      phammerLexer->T__27(phammerLexer)
 *  -  void      phammerLexer->T__28(phammerLexer)
 *  -  void      phammerLexer->T__29(phammerLexer)
 *  -  void      phammerLexer->T__30(phammerLexer)
 *  -  void      phammerLexer->T__31(phammerLexer)
 *  -  void      phammerLexer->T__32(phammerLexer)
 *  -  void      phammerLexer->T__33(phammerLexer)
 *  -  void      phammerLexer->T__34(phammerLexer)
 *  -  void      phammerLexer->T__35(phammerLexer)
 *  -  void      phammerLexer->T__36(phammerLexer)
 *  -  void      phammerLexer->SLASH(phammerLexer)
 *  -  void      phammerLexer->PUBLIC_TAG(phammerLexer)
 *  -  void      phammerLexer->ID(phammerLexer)
 *  -  void      phammerLexer->COLON(phammerLexer)
 *  -  void      phammerLexer->STRING(phammerLexer)
 *  -  void      phammerLexer->STRING_ID(phammerLexer)
 *  -  void      phammerLexer->COMMENT(phammerLexer)
 *  -  void      phammerLexer->WS(phammerLexer)
 *  -  void      phammerLexer->Tokens(phammerLexer)
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
#ifndef	_hammerLexer_H
#define _hammerLexer_H
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
typedef struct hammerLexer_Ctx_struct hammerLexer, * phammerLexer;



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


/** Context tracking structure for hammerLexer
 */
struct hammerLexer_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_LEXER    pLexer;


     void (*mT__27)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__28)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__29)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__30)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__31)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__32)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__33)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__34)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__35)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mT__36)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mSLASH)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mPUBLIC_TAG)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mID)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mCOLON)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mSTRING)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mSTRING_ID)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mCOMMENT)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mWS)	(struct hammerLexer_Ctx_struct * ctx);
     void (*mTokens)	(struct hammerLexer_Ctx_struct * ctx);    const char * (*getGrammarFileName)();
    void	    (*free)   (struct hammerLexer_Ctx_struct * ctx);
        
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API phammerLexer hammerLexerNew         (pANTLR3_INPUT_STREAM instream);
ANTLR3_API phammerLexer hammerLexerNewSSD      (pANTLR3_INPUT_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

/** Symbolic definitions of all the tokens that the lexer will work with.
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
#define TARGET_REF      8
#define T__29      29
#define T__28      28
#define ARGUMENTS      11
#define T__27      27
#define TARGET_NAME      9
#define STRING_ID      25
#define LIST_OF      14
#define EMPTY_EXPRESSION      13
#define IMPLICIT_PROJECT_DEF      6
#define ID      20
#define EOF      -1
#define REQUIREMENT      17
#define COLON      21
#define T__30      30
#define T__31      31
#define T__32      32
#define CONDITIONAL_REQUIREMENT      18
#define T__33      33
#define WS      19
#define SLASH      22
#define TARGET_DECL_OR_RULE_CALL      7
#define T__34      34
#define EMPTY_TARGET_NAME      10
#define T__35      35
#define T__36      36
#define EXPLICIT_PROJECT_DEF      5
#define PUBLIC_TAG      23
#define HAMFILE      4
#define PATH_LIKE_SEQ      15
#define REQUIREMENT_SET      16
#define COMMENT      26
#define EXPRESSION      12
#define STRING      24
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for hammerLexer
 * =============================================================================
 */
/** \} */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
