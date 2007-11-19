/** \file
 *  This C source file was generated by $ANTLR version 3.0.1
 *
 *     -  From the grammar source file : hammer.g
 *     -                            On : 2007-11-16 18:19:04
 *     -                for the parser : hammerParserParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 * View this file with tabs set to 8 (:set ts=8 in gvim) and indent at 4 (:set sw=4 in gvim)
 *
*/
/* -----------------------------------------
 * Include the ANTLR3 generated header file.
 */
#include    "hammerParser.h"
/* ----------------------------------------- */



/* Aids in accessing scopes for grammar programmers
 */
#undef	SCOPE_TYPE
#undef	SCOPE_STACK
#undef	SCOPE_TOP
#define	SCOPE_TYPE(scope)   phammerParser_##scope##_SCOPE
#define SCOPE_STACK(scope)  phammerParser_##scope##Stack
#define	SCOPE_TOP(scope)    ctx->phammerParser_##scope##Top
#define	SCOPE_SIZE(scope)			(ctx->SCOPE_STACK(scope)->size(ctx->SCOPE_STACK(scope)))
#define SCOPE_INSTANCE(scope, i)	(ctx->SCOPE_STACK(scope)->get(ctx->SCOPE_STACK(scope),i))

/* MACROS that hide the C interface implementations from the
 * generated code, which makes it a little more understandable to the human eye.
 * I am very much against using C pre-processor macros for function calls and bits
 * of code as you cannot see what is happening when single stepping in debuggers
 * and so on. The exception (in my book at least) is for generated code, where you are
 * not maintaining it, but may wish to read and understand it. If you single step it, you know that input()
 * hides some indirect calls, but is always refering to the input stream. This is
 * probably more readable than ctx->input->istream->input(snarfle0->blarg) and allows me to rejig
 * the runtime interfaces without changing the generated code too often, without
 * confusing the reader of the generated output, who may not wish to know the gory
 * details of the interface inheritence.
 */
/* Macros for accessing things in the parser
 */
 
#undef	    PARSER		    
#undef	    RECOGNIZER		    
#undef	    HAVEPARSEDRULE
#undef		MEMOIZE
#undef	    INPUT
#undef	    STRSTREAM
#undef	    HASEXCEPTION
#undef	    EXCEPTION
#undef	    MATCHT
#undef	    MATCHANYT
#undef	    FOLLOWSTACK
#undef	    FOLLOWPUSH
#undef	    FOLLOWPOP
#undef	    PRECOVER
#undef	    PREPORTERROR
#undef	    LA
#undef	    LT
#undef	    CONSTRUCTEX
#undef	    CONSUME
#undef	    MARK
#undef	    REWIND
#undef	    REWINDLAST
#undef	    PERRORRECOVERY
#undef	    HASFAILED
#undef	    FAILEDFLAG
#undef	    RECOVERFROMMISMATCHEDSET
#undef	    RECOVERFROMMISMATCHEDELEMENT
#undef		INDEX
#undef      ADAPTOR
#undef		SEEK

#define	    PARSER							ctx->pParser  
#define	    RECOGNIZER						PARSER->rec
#define	    HAVEPARSEDRULE(r)				RECOGNIZER->alreadyParsedRule(RECOGNIZER, r)
#define		MEMOIZE(ri,si)					RECOGNIZER->memoize(RECOGNIZER, ri, si)
#define	    INPUT							PARSER->tstream
#define	    STRSTREAM						INPUT
#define		INDEX()							INPUT->istream->index(INPUT->istream)
#define	    HASEXCEPTION()					(RECOGNIZER->error == ANTLR3_TRUE)
#define	    EXCEPTION						RECOGNIZER->exception
#define	    MATCHT(t, fs)					RECOGNIZER->match(RECOGNIZER, t, fs)
#define	    MATCHANYT()						RECOGNIZER->matchAny(RECOGNIZER)
#define	    FOLLOWSTACK					    RECOGNIZER->following
#define	    FOLLOWPUSH(x)					FOLLOWSTACK->push(FOLLOWSTACK, ((void *)(&(x))), NULL)
#define	    FOLLOWPOP()						FOLLOWSTACK->pop(FOLLOWSTACK)
#define	    PRECOVER()						RECOGNIZER->recover(RECOGNIZER)
#define	    PREPORTERROR()					RECOGNIZER->reportError(RECOGNIZER)
#define	    LA(n)							INPUT->istream->_LA(INPUT->istream, n)
#define	    LT(n)							INPUT->_LT(INPUT, n)
#define	    CONSTRUCTEX()					RECOGNIZER->exConstruct(RECOGNIZER)
#define	    CONSUME()						INPUT->istream->consume(INPUT->istream)
#define	    MARK()							INPUT->istream->mark(INPUT->istream)
#define	    REWIND(m)						INPUT->istream->rewind(INPUT->istream, m)
#define	    REWINDLAST()					INPUT->istream->rewindLast(INPUT->istream)
#define		SEEK(n)							INPUT->istream->seek(INPUT->istream, n)
#define	    PERRORRECOVERY					RECOGNIZER->errorRecovery
#define	    _fsp							RECOGNIZER->_fsp
#define	    FAILEDFLAG						RECOGNIZER->failed
#define	    HASFAILED()						(FAILEDFLAG == ANTLR3_TRUE)
#define	    BACKTRACKING					RECOGNIZER->backtracking
#define	    RECOVERFROMMISMATCHEDSET(s)		RECOGNIZER->recoverFromMismatchedSet(RECOGNIZER, s)
#define	    RECOVERFROMMISMATCHEDELEMENT(e)	RECOGNIZER->recoverFromMismatchedElement(RECOGNIZER, s)
#define     ADAPTOR                         ctx->adaptor
#define		TOKTEXT(tok, txt)				tok, (pANTLR3_UINT8)txt

/* The 4 tokens defined below may well clash with your own #defines or token types. If so
 * then for the present you must use different names for your defines as these are hard coded
 * in the code generator. It would be better not to use such names internally, and maybe
 * we can change this in a forthcoming release. I deliberately do not #undef these
 * here as this will at least give you a redefined error somewhere if they clash.
 */
#define	    UP	    ANTLR3_TOKEN_UP
#define	    DOWN    ANTLR3_TOKEN_DOWN
#define	    EOR	    ANTLR3_TOKEN_EOR
#define	    INVALID ANTLR3_TOKEN_INVALID


/* =============================================================================
 * Functions to create and destroy scopes. First come the rule scopes, followed
 * by the global declared scopes.
 */



/* ============================================================================= */

/* =============================================================================
 * Start of recognizer
 */


/** \brief Table of all token names in symbolic order, mainly used for
 *         error reporting.
 */
static pANTLR3_UINT8   hammerParserTokenNames[]
     = {
        (pANTLR3_UINT8) "<invalid>",       /* String to print to indicate an invalid token */
        (pANTLR3_UINT8) "<EOR>",
        (pANTLR3_UINT8) "<DOWN>", 
        (pANTLR3_UINT8) "<UP>", 
        (pANTLR3_UINT8) "ID",
        (pANTLR3_UINT8) "STRING",
        (pANTLR3_UINT8) "COMMENT",
        (pANTLR3_UINT8) "WS",
        (pANTLR3_UINT8) "BIG_COMMENT",
        (pANTLR3_UINT8) "';'",
        (pANTLR3_UINT8) "':'",
        (pANTLR3_UINT8) "'<'",
        (pANTLR3_UINT8) "'>'"
       };


        

/* Forward declare the locally static matching functions we have generated.
 */
static hammerParser_rules_return	rules    (phammerParser ctx);
static hammerParser_rule_return	rule    (phammerParser ctx);
static hammerParser_rule_args_return	rule_args    (phammerParser ctx);
static hammerParser_rule_posible_args_return	rule_posible_args    (phammerParser ctx);
static hammerParser_feature_list_return	feature_list    (phammerParser ctx);
static hammerParser_feature_return	feature    (phammerParser ctx);
static hammerParser_string_list_return	string_list    (phammerParser ctx);
static void	hammerParserFree(phammerParser ctx);

/* Function to initialize bitset APIs
 */
static	void hammerParserLoadFollowSets();

/* For use in tree output where we are accumulating rule labels via label += ruleRef
 * we need a function that knows how to free a return scope when the list is destroyed. 
 * We cannot just use ANTLR3_FREE because in debug tracking mode, this is a macro.
 */
static	void ANTLR3_CDECL freeScope(void * scope)
{
    ANTLR3_FREE(scope);
}

/** \brief Name of the gramar file that generated this code
 */
static unsigned char fileName[] = "hammer.g";

/** \brief Return the name of the grammar file that generated this code.
 */
static unsigned char * getGrammarFileName()
{
	return fileName;
}
/** \brief Create a new hammerParser parser and retrun a context for it.
 *
 * \param[in] instream Pointer to an input stream interface.
 *
 * \return Pointer to new parser context upon success.
 */
ANTLR3_API phammerParser
hammerParserNew   (pANTLR3_COMMON_TOKEN_STREAM instream)
{
    phammerParser ctx;	    /* Context structure we will build and return   */
    
    ctx	= (phammerParser) ANTLR3_MALLOC(sizeof(hammerParser));
    
    if	(ctx == NULL)
    {
	/* Failed to allocate memory for parser context */
        return  (phammerParser)ANTLR3_ERR_NOMEM;
    }
    
    /* -------------------------------------------------------------------
     * Memory for basic structure is allocated, now to fill in
     * the base ANTLR3 structures. We intialize the function pointers
     * for the standard ANTLR3 parser function set, but upon return
     * from here, the programmer may set the pointers to provide custom
     * implementations of each function. 
     *
     * We don't use the macros defined in hammerParser.h here, in order that you can get a sense
     * of what goes where.
     */

    /* Create a base parser/recognizer, using the supplied token stream
     */
    ctx->pParser	    = antlr3ParserNewStream(ANTLR3_SIZE_HINT, instream->tstream);
    /* Install the implementation of our hammerParser interface
     */
    ctx->rules	= rules;
    ctx->rule	= rule;
    ctx->rule_args	= rule_args;
    ctx->rule_posible_args	= rule_posible_args;
    ctx->feature_list	= feature_list;
    ctx->feature	= feature;
    ctx->string_list	= string_list;

    ctx->free			= hammerParserFree;
    ctx->getGrammarFileName	= getGrammarFileName;
    
    /* Install the scope pushing methods.
     */
    ADAPTOR	= ANTLR3_TREE_ADAPTORNew(instream->tstream->tokenSource->strFactory);
    ctx->vectors	= antlr3VectorFactoryNew(64);
    
	
    /* Install the token table
     */
    RECOGNIZER->tokenNames   = hammerParserTokenNames;
    
    /* Initialize the follow bit sets
     */
    hammerParserLoadFollowSets();
    
    /* Return the newly built parser to the caller
     */
    return  ctx;
}

/** Free the parser resources
 */
 static void
 hammerParserFree(phammerParser ctx)
 {
    /* Free any scope memory
     */
    
    ctx->vectors->close(ctx->vectors);
    /* We created the adaptor so we must free it
     */
    ADAPTOR->free(ADAPTOR);
    ctx->pParser->free(ctx->pParser);
    ANTLR3_FREE(ctx);

    /* Everything is released, so we can return
     */
    return;
 }
 
/** Return token names used by this parser
 *
 * The returned pointer is used as an index into the token names table (using the token 
 * number as the index).
 * 
 * \return Pointer to first char * in the table.
 */
static pANTLR3_UINT8    *getTokenNames() 
{
        return hammerParserTokenNames; 
}

    
/* Declare the bitsets
 */

/** Bitset defining follow set for error recovery in rule state: FOLLOW_rule_in_rules29  */
static	ANTLR3_BITWORD FOLLOW_rule_in_rules29_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000012) };
static  ANTLR3_BITSET FOLLOW_rule_in_rules29	= { FOLLOW_rule_in_rules29_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_ID_in_rule38  */
static	ANTLR3_BITWORD FOLLOW_ID_in_rule38_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000E10) };
static  ANTLR3_BITSET FOLLOW_ID_in_rule38	= { FOLLOW_ID_in_rule38_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_rule_args_in_rule40  */
static	ANTLR3_BITWORD FOLLOW_rule_args_in_rule40_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000200) };
static  ANTLR3_BITSET FOLLOW_rule_args_in_rule40	= { FOLLOW_rule_args_in_rule40_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_9_in_rule42  */
static	ANTLR3_BITWORD FOLLOW_9_in_rule42_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET FOLLOW_9_in_rule42	= { FOLLOW_9_in_rule42_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_rule_posible_args_in_rule_args51  */
static	ANTLR3_BITWORD FOLLOW_rule_posible_args_in_rule_args51_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000402) };
static  ANTLR3_BITSET FOLLOW_rule_posible_args_in_rule_args51	= { FOLLOW_rule_posible_args_in_rule_args51_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_10_in_rule_args54  */
static	ANTLR3_BITWORD FOLLOW_10_in_rule_args54_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000C12) };
static  ANTLR3_BITSET FOLLOW_10_in_rule_args54	= { FOLLOW_10_in_rule_args54_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_rule_posible_args_in_rule_args56  */
static	ANTLR3_BITWORD FOLLOW_rule_posible_args_in_rule_args56_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000402) };
static  ANTLR3_BITSET FOLLOW_rule_posible_args_in_rule_args56	= { FOLLOW_rule_posible_args_in_rule_args56_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_feature_list_in_rule_posible_args67  */
static	ANTLR3_BITWORD FOLLOW_feature_list_in_rule_posible_args67_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET FOLLOW_feature_list_in_rule_posible_args67	= { FOLLOW_feature_list_in_rule_posible_args67_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_string_list_in_rule_posible_args71  */
static	ANTLR3_BITWORD FOLLOW_string_list_in_rule_posible_args71_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET FOLLOW_string_list_in_rule_posible_args71	= { FOLLOW_string_list_in_rule_posible_args71_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_feature_in_feature_list79  */
static	ANTLR3_BITWORD FOLLOW_feature_in_feature_list79_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000802) };
static  ANTLR3_BITSET FOLLOW_feature_in_feature_list79	= { FOLLOW_feature_in_feature_list79_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_11_in_feature88  */
static	ANTLR3_BITWORD FOLLOW_11_in_feature88_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000010) };
static  ANTLR3_BITSET FOLLOW_11_in_feature88	= { FOLLOW_11_in_feature88_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_ID_in_feature90  */
static	ANTLR3_BITWORD FOLLOW_ID_in_feature90_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000001000) };
static  ANTLR3_BITSET FOLLOW_ID_in_feature90	= { FOLLOW_ID_in_feature90_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_12_in_feature92  */
static	ANTLR3_BITWORD FOLLOW_12_in_feature92_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000010) };
static  ANTLR3_BITSET FOLLOW_12_in_feature92	= { FOLLOW_12_in_feature92_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_ID_in_feature94  */
static	ANTLR3_BITWORD FOLLOW_ID_in_feature94_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET FOLLOW_ID_in_feature94	= { FOLLOW_ID_in_feature94_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_ID_in_string_list102  */
static	ANTLR3_BITWORD FOLLOW_ID_in_string_list102_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000012) };
static  ANTLR3_BITSET FOLLOW_ID_in_string_list102	= { FOLLOW_ID_in_string_list102_bits, 1	};
     
/** Load up the static bitsets for following set for error recovery.
 *  \remark
 *  These are static after the parser is generated, hence they are static
 *  delcarations in the parser and are thread safe after initialization.
 */
static
void hammerParserLoadFollowSets()
{
    antlr3BitsetSetAPI(&FOLLOW_rule_in_rules29);
    antlr3BitsetSetAPI(&FOLLOW_ID_in_rule38);
    antlr3BitsetSetAPI(&FOLLOW_rule_args_in_rule40);
    antlr3BitsetSetAPI(&FOLLOW_9_in_rule42);
    antlr3BitsetSetAPI(&FOLLOW_rule_posible_args_in_rule_args51);
    antlr3BitsetSetAPI(&FOLLOW_10_in_rule_args54);
    antlr3BitsetSetAPI(&FOLLOW_rule_posible_args_in_rule_args56);
    antlr3BitsetSetAPI(&FOLLOW_feature_list_in_rule_posible_args67);
    antlr3BitsetSetAPI(&FOLLOW_string_list_in_rule_posible_args71);
    antlr3BitsetSetAPI(&FOLLOW_feature_in_feature_list79);
    antlr3BitsetSetAPI(&FOLLOW_11_in_feature88);
    antlr3BitsetSetAPI(&FOLLOW_ID_in_feature90);
    antlr3BitsetSetAPI(&FOLLOW_12_in_feature92);
    antlr3BitsetSetAPI(&FOLLOW_ID_in_feature94);
    antlr3BitsetSetAPI(&FOLLOW_ID_in_string_list102);

    return;
}

 
 
/* ==============================================
 * Parsing rules
 */
/** 
 * $ANTLR start rules
 * hammer.g:5:1: rules : ( rule )* ;
 */
static hammerParser_rules_return
rules(phammerParser ctx)
{   
    hammerParser_rules_return retval;

    pANTLR3_BASE_TREE root_0;

    hammerParser_rule_return rule1;
    #undef	RETURN_TYPE_rule1
    #define	RETURN_TYPE_rule1 hammerParser_rule_return


    /* Initialize rule variables
     */


    root_0 = NULL;

    rule1.tree = NULL;

    retval.start = LT(1);

    retval.tree  = NULL;
    {
        // hammer.g:5:7: ( ( rule )* )
        // hammer.g:5:10: ( rule )*
        {
            root_0 = ADAPTOR->nil(ADAPTOR);


            // hammer.g:5:10: ( rule )*

            for (;;)
            {
                int alt1=2;
                {
                   /* dfaLoopbackState(k,edges,eotPredictsAlt,description,stateNumber,semPredState)
                    */
                    int LA1_0 = LA(1);
                    if ( (LA1_0 == ID) ) 
                    {
                        alt1=1;
                    }

                }
                switch (alt1) 
                {
            	case 1:
            	    // hammer.g:5:10: rule
            	    {
            	        FOLLOWPUSH(FOLLOW_rule_in_rules29);
            	        rule1=rule(ctx);
            	        FOLLOWPOP();
            	        if  (HASEXCEPTION())
            	        {
            	            goto rulerulesEx;
            	        }

            	        ADAPTOR->addChild(ADAPTOR, root_0, rule1.tree);

            	    }
            	    break;

            	default:
            	    goto loop1;	/* break out of the loop */
            	    break;
                }
            }
            loop1: ; /* Jump out to here if this rule does not match */


        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulerulesEx; /* Prevent compiler warnings */
    rulerulesEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end rules */

/** 
 * $ANTLR start rule
 * hammer.g:6:1: rule : ID rule_args ';' ;
 */
static hammerParser_rule_return
rule(phammerParser ctx)
{   
    hammerParser_rule_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    ID2;
    pANTLR3_COMMON_TOKEN    char_literal4;
    hammerParser_rule_args_return rule_args3;
    #undef	RETURN_TYPE_rule_args3
    #define	RETURN_TYPE_rule_args3 hammerParser_rule_args_return

    pANTLR3_BASE_TREE ID2_tree;
    pANTLR3_BASE_TREE char_literal4_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    ID2       = NULL;
    char_literal4       = NULL;
    rule_args3.tree = NULL;

    retval.start = LT(1);

    ID2_tree   = NULL;
    char_literal4_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:6:7: ( ID rule_args ';' )
        // hammer.g:6:9: ID rule_args ';'
        {
            root_0 = ADAPTOR->nil(ADAPTOR);

            ID2 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(ID, &FOLLOW_ID_in_rule38); 

            ID2_tree = ADAPTOR->create(ADAPTOR, ID2);
            ADAPTOR->addChild(ADAPTOR, root_0, ID2_tree);

            FOLLOWPUSH(FOLLOW_rule_args_in_rule40);
            rule_args3=rule_args(ctx);
            FOLLOWPOP();
            if  (HASEXCEPTION())
            {
                goto ruleruleEx;
            }

            ADAPTOR->addChild(ADAPTOR, root_0, rule_args3.tree);
            char_literal4 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(9, &FOLLOW_9_in_rule42); 

            char_literal4_tree = ADAPTOR->create(ADAPTOR, char_literal4);
            ADAPTOR->addChild(ADAPTOR, root_0, char_literal4_tree);


        }

    }
    

    // This is where rules clean up and exit
    //
    goto ruleruleEx; /* Prevent compiler warnings */
    ruleruleEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end rule */

/** 
 * $ANTLR start rule_args
 * hammer.g:7:1: rule_args : rule_posible_args ( ':' rule_posible_args )* ;
 */
static hammerParser_rule_args_return
rule_args(phammerParser ctx)
{   
    hammerParser_rule_args_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    char_literal6;
    hammerParser_rule_posible_args_return rule_posible_args5;
    #undef	RETURN_TYPE_rule_posible_args5
    #define	RETURN_TYPE_rule_posible_args5 hammerParser_rule_posible_args_return

    hammerParser_rule_posible_args_return rule_posible_args7;
    #undef	RETURN_TYPE_rule_posible_args7
    #define	RETURN_TYPE_rule_posible_args7 hammerParser_rule_posible_args_return

    pANTLR3_BASE_TREE char_literal6_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    char_literal6       = NULL;
    rule_posible_args5.tree = NULL;

    rule_posible_args7.tree = NULL;

    retval.start = LT(1);

    char_literal6_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:7:12: ( rule_posible_args ( ':' rule_posible_args )* )
        // hammer.g:7:14: rule_posible_args ( ':' rule_posible_args )*
        {
            root_0 = ADAPTOR->nil(ADAPTOR);

            FOLLOWPUSH(FOLLOW_rule_posible_args_in_rule_args51);
            rule_posible_args5=rule_posible_args(ctx);
            FOLLOWPOP();
            if  (HASEXCEPTION())
            {
                goto rulerule_argsEx;
            }

            ADAPTOR->addChild(ADAPTOR, root_0, rule_posible_args5.tree);

            // hammer.g:7:32: ( ':' rule_posible_args )*

            for (;;)
            {
                int alt2=2;
                {
                   /* dfaLoopbackState(k,edges,eotPredictsAlt,description,stateNumber,semPredState)
                    */
                    int LA2_0 = LA(1);
                    if ( (LA2_0 == 10) ) 
                    {
                        alt2=1;
                    }

                }
                switch (alt2) 
                {
            	case 1:
            	    // hammer.g:7:33: ':' rule_posible_args
            	    {
            	        char_literal6 = (pANTLR3_COMMON_TOKEN)LT(1);
            	        MATCHT(10, &FOLLOW_10_in_rule_args54); 

            	        char_literal6_tree = ADAPTOR->create(ADAPTOR, char_literal6);
            	        ADAPTOR->addChild(ADAPTOR, root_0, char_literal6_tree);

            	        FOLLOWPUSH(FOLLOW_rule_posible_args_in_rule_args56);
            	        rule_posible_args7=rule_posible_args(ctx);
            	        FOLLOWPOP();
            	        if  (HASEXCEPTION())
            	        {
            	            goto rulerule_argsEx;
            	        }

            	        ADAPTOR->addChild(ADAPTOR, root_0, rule_posible_args7.tree);

            	    }
            	    break;

            	default:
            	    goto loop2;	/* break out of the loop */
            	    break;
                }
            }
            loop2: ; /* Jump out to here if this rule does not match */


        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulerule_argsEx; /* Prevent compiler warnings */
    rulerule_argsEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end rule_args */

/** 
 * $ANTLR start rule_posible_args
 * hammer.g:8:1: rule_posible_args : ( feature_list | string_list ) ;
 */
static hammerParser_rule_posible_args_return
rule_posible_args(phammerParser ctx)
{   
    hammerParser_rule_posible_args_return retval;

    pANTLR3_BASE_TREE root_0;

    hammerParser_feature_list_return feature_list8;
    #undef	RETURN_TYPE_feature_list8
    #define	RETURN_TYPE_feature_list8 hammerParser_feature_list_return

    hammerParser_string_list_return string_list9;
    #undef	RETURN_TYPE_string_list9
    #define	RETURN_TYPE_string_list9 hammerParser_string_list_return


    /* Initialize rule variables
     */


    root_0 = NULL;

    feature_list8.tree = NULL;

    string_list9.tree = NULL;

    retval.start = LT(1);

    retval.tree  = NULL;
    {
        // hammer.g:8:19: ( ( feature_list | string_list ) )
        // hammer.g:8:21: ( feature_list | string_list )
        {
            root_0 = ADAPTOR->nil(ADAPTOR);


            // hammer.g:8:21: ( feature_list | string_list )
            {
                int alt3=2;
                switch ( LA(1) ) 
                {
                case 11:
                	{
                		alt3=1;
                	}
                    break;
                case 10:
                	{
                		alt3=1;
                	}
                    break;
                case 9:
                	{
                		alt3=1;
                	}
                    break;
                case ID:
                	{
                		alt3=2;
                	}
                    break;

                default:
                    CONSTRUCTEX();
                    EXCEPTION->type         = ANTLR3_NO_VIABLE_ALT_EXCEPTION;
                    EXCEPTION->message      = "8:21: ( feature_list | string_list )";
                    EXCEPTION->decisionNum  = 3;
                    EXCEPTION->state        = 0;


                    goto rulerule_posible_argsEx;
                }

                switch (alt3) 
                {
            	case 1:
            	    // hammer.g:8:22: feature_list
            	    {
            	        FOLLOWPUSH(FOLLOW_feature_list_in_rule_posible_args67);
            	        feature_list8=feature_list(ctx);
            	        FOLLOWPOP();
            	        if  (HASEXCEPTION())
            	        {
            	            goto rulerule_posible_argsEx;
            	        }

            	        ADAPTOR->addChild(ADAPTOR, root_0, feature_list8.tree);

            	    }
            	    break;
            	case 2:
            	    // hammer.g:8:37: string_list
            	    {
            	        FOLLOWPUSH(FOLLOW_string_list_in_rule_posible_args71);
            	        string_list9=string_list(ctx);
            	        FOLLOWPOP();
            	        if  (HASEXCEPTION())
            	        {
            	            goto rulerule_posible_argsEx;
            	        }

            	        ADAPTOR->addChild(ADAPTOR, root_0, string_list9.tree);

            	    }
            	    break;

                }
            }

        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulerule_posible_argsEx; /* Prevent compiler warnings */
    rulerule_posible_argsEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end rule_posible_args */

/** 
 * $ANTLR start feature_list
 * hammer.g:9:1: feature_list : ( feature )* ;
 */
static hammerParser_feature_list_return
feature_list(phammerParser ctx)
{   
    hammerParser_feature_list_return retval;

    pANTLR3_BASE_TREE root_0;

    hammerParser_feature_return feature10;
    #undef	RETURN_TYPE_feature10
    #define	RETURN_TYPE_feature10 hammerParser_feature_return


    /* Initialize rule variables
     */


    root_0 = NULL;

    feature10.tree = NULL;

    retval.start = LT(1);

    retval.tree  = NULL;
    {
        // hammer.g:9:14: ( ( feature )* )
        // hammer.g:9:16: ( feature )*
        {
            root_0 = ADAPTOR->nil(ADAPTOR);


            // hammer.g:9:16: ( feature )*

            for (;;)
            {
                int alt4=2;
                {
                   /* dfaLoopbackState(k,edges,eotPredictsAlt,description,stateNumber,semPredState)
                    */
                    int LA4_0 = LA(1);
                    if ( (LA4_0 == 11) ) 
                    {
                        alt4=1;
                    }

                }
                switch (alt4) 
                {
            	case 1:
            	    // hammer.g:9:16: feature
            	    {
            	        FOLLOWPUSH(FOLLOW_feature_in_feature_list79);
            	        feature10=feature(ctx);
            	        FOLLOWPOP();
            	        if  (HASEXCEPTION())
            	        {
            	            goto rulefeature_listEx;
            	        }

            	        ADAPTOR->addChild(ADAPTOR, root_0, feature10.tree);

            	    }
            	    break;

            	default:
            	    goto loop4;	/* break out of the loop */
            	    break;
                }
            }
            loop4: ; /* Jump out to here if this rule does not match */


        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulefeature_listEx; /* Prevent compiler warnings */
    rulefeature_listEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end feature_list */

/** 
 * $ANTLR start feature
 * hammer.g:10:1: feature : '<' ID '>' ID ;
 */
static hammerParser_feature_return
feature(phammerParser ctx)
{   
    hammerParser_feature_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    char_literal11;
    pANTLR3_COMMON_TOKEN    ID12;
    pANTLR3_COMMON_TOKEN    char_literal13;
    pANTLR3_COMMON_TOKEN    ID14;

    pANTLR3_BASE_TREE char_literal11_tree;
    pANTLR3_BASE_TREE ID12_tree;
    pANTLR3_BASE_TREE char_literal13_tree;
    pANTLR3_BASE_TREE ID14_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    char_literal11       = NULL;
    ID12       = NULL;
    char_literal13       = NULL;
    ID14       = NULL;
    retval.start = LT(1);

    char_literal11_tree   = NULL;
    ID12_tree   = NULL;
    char_literal13_tree   = NULL;
    ID14_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:10:10: ( '<' ID '>' ID )
        // hammer.g:10:12: '<' ID '>' ID
        {
            root_0 = ADAPTOR->nil(ADAPTOR);

            char_literal11 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(11, &FOLLOW_11_in_feature88); 

            char_literal11_tree = ADAPTOR->create(ADAPTOR, char_literal11);
            ADAPTOR->addChild(ADAPTOR, root_0, char_literal11_tree);

            ID12 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(ID, &FOLLOW_ID_in_feature90); 

            ID12_tree = ADAPTOR->create(ADAPTOR, ID12);
            ADAPTOR->addChild(ADAPTOR, root_0, ID12_tree);

            char_literal13 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(12, &FOLLOW_12_in_feature92); 

            char_literal13_tree = ADAPTOR->create(ADAPTOR, char_literal13);
            ADAPTOR->addChild(ADAPTOR, root_0, char_literal13_tree);

            ID14 = (pANTLR3_COMMON_TOKEN)LT(1);
            MATCHT(ID, &FOLLOW_ID_in_feature94); 

            ID14_tree = ADAPTOR->create(ADAPTOR, ID14);
            ADAPTOR->addChild(ADAPTOR, root_0, ID14_tree);


        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulefeatureEx; /* Prevent compiler warnings */
    rulefeatureEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end feature */

/** 
 * $ANTLR start string_list
 * hammer.g:11:1: string_list : ( ID )* ;
 */
static hammerParser_string_list_return
string_list(phammerParser ctx)
{   
    hammerParser_string_list_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    ID15;

    pANTLR3_BASE_TREE ID15_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    ID15       = NULL;
    retval.start = LT(1);

    ID15_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:11:13: ( ( ID )* )
        // hammer.g:11:15: ( ID )*
        {
            root_0 = ADAPTOR->nil(ADAPTOR);


            // hammer.g:11:15: ( ID )*

            for (;;)
            {
                int alt5=2;
                {
                   /* dfaLoopbackState(k,edges,eotPredictsAlt,description,stateNumber,semPredState)
                    */
                    int LA5_0 = LA(1);
                    if ( (LA5_0 == ID) ) 
                    {
                        alt5=1;
                    }

                }
                switch (alt5) 
                {
            	case 1:
            	    // hammer.g:11:15: ID
            	    {
            	        ID15 = (pANTLR3_COMMON_TOKEN)LT(1);
            	        MATCHT(ID, &FOLLOW_ID_in_string_list102); 

            	        ID15_tree = ADAPTOR->create(ADAPTOR, ID15);
            	        ADAPTOR->addChild(ADAPTOR, root_0, ID15_tree);


            	    }
            	    break;

            	default:
            	    goto loop5;	/* break out of the loop */
            	    break;
                }
            }
            loop5: ; /* Jump out to here if this rule does not match */


        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulestring_listEx; /* Prevent compiler warnings */
    rulestring_listEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = ADAPTOR->rulePostProcessing(ADAPTOR, root_0);
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
    }

    return retval;
}
/* $ANTLR end string_list */
/* End of parsing rules
 * ==============================================
 */

/* ==============================================
 * Syntactic predicates
 */
/* End of syntactic predicates
 * ==============================================
 */

 
 



/* End of code
 * =============================================================================
 */
