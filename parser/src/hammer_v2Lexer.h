/** \file
 *  This C header file was generated by $ANTLR version 3.1.1
 *
 *     -  From the grammar source file : hammer_v2.g
 *     -                            On : 2019-04-21 20:08:21
 *     -                 for the lexer : hammer_v2LexerLexer *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The lexer hammer_v2Lexer has the callable functions (rules) shown below,
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
 * a parser context typedef phammer_v2Lexer, which is returned from a call to hammer_v2LexerNew().
 *
 * As this is a generated lexer, it is unlikely you will call it 'manually'. However
 * the methods are provided anyway.
 * * The methods in phammer_v2Lexer are  as follows:
 *
 *  -  void      phammer_v2Lexer->T__42(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__43(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__44(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__45(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__46(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__47(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__48(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__49(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__50(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__51(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__52(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->T__53(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->PathUri(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Slash(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->DoubleSlash(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->PublicTag(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Local(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Explicit(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->QuestionMark(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Asterix(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->DoubleAsterix(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->ProjectLocalRefTag(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->LogicalOr(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->LogicalAnd(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Id(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->STRING(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->STRING_ID(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->STRING_1(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->STRING_ID1(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->COMMENT(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->WS(phammer_v2Lexer)
 *  -  void      phammer_v2Lexer->Tokens(phammer_v2Lexer)
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
#ifndef	_hammer_v2Lexer_H
#define _hammer_v2Lexer_H
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
typedef struct hammer_v2Lexer_Ctx_struct hammer_v2Lexer, * phammer_v2Lexer;



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


/** Context tracking structure for hammer_v2Lexer
 */
struct hammer_v2Lexer_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_LEXER    pLexer;


     void (*mT__42)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__43)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__44)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__45)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__46)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__47)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__48)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__49)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__50)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__51)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__52)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mT__53)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mPathUri)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mSlash)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mDoubleSlash)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mPublicTag)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mLocal)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mExplicit)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mQuestionMark)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mAsterix)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mDoubleAsterix)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mProjectLocalRefTag)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mLogicalOr)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mLogicalAnd)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mId)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mSTRING)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mSTRING_ID)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mSTRING_1)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mSTRING_ID1)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mCOMMENT)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mWS)	(struct hammer_v2Lexer_Ctx_struct * ctx);
     void (*mTokens)	(struct hammer_v2Lexer_Ctx_struct * ctx);    const char * (*getGrammarFileName)();
    void	    (*free)   (struct hammer_v2Lexer_Ctx_struct * ctx);
        
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API phammer_v2Lexer hammer_v2LexerNew         (pANTLR3_INPUT_STREAM instream);
ANTLR3_API phammer_v2Lexer hammer_v2LexerNewSSD      (pANTLR3_INPUT_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

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
#define T__50      50
#define Hamfile      4
#define EmptyField      12
#define Explicit      7
#define PathTrailingSlash      20
#define Feature      15
#define Wildcard      24
#define Slash      28
#define List      14
#define T__51      51
#define NamedArgument      10
#define DoubleAsterix      34
#define T__52      52
#define T__53      53
#define STRING_1      38
#define Arguments      8
#define Local      6
#define EmptyArgument      9
#define Asterix      33
#define Rule      5
#define Id      26
#define RuleInvocation      17
#define ProjectLocalRefTag      29
#define LogicalOr      35
#define COMMENT      41
#define NamedField      13
#define PathRootName      19
#define LogicalAnd      36
#define STRING_ID      39
#define Structure      11
#define WS      25
#define DoubleSlash      31
#define EOF      -1
#define Condition      16
#define Path      18
#define PublicTag      27
#define TargetRefSpec      22
#define PathUri      30
#define STRING_ID1      40
#define T__48      48
#define T__49      49
#define QuestionMark      32
#define TargetRef      21
#define T__44      44
#define T__45      45
#define STRING      37
#define T__46      46
#define T__47      47
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

/* End of token definitions for hammer_v2Lexer
 * =============================================================================
 */
/** \} */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
