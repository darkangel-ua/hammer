/** \file
 *  This C source file was generated by $ANTLR version 3.1.1
 *
 *     -  From the grammar source file : hammer.g
 *     -                            On : 2010-06-27 18:46:25
 *     -                for the parser : hammerParserParser *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
*/
/* -----------------------------------------
 * Include the ANTLR3 generated header file.
 */
#include    "hammerParser.h"
/* ----------------------------------------- */





/* MACROS that hide the C interface implementations from the
 * generated code, which makes it a little more understandable to the human eye.
 * I am very much against using C pre-processor macros for function calls and bits
 * of code as you cannot see what is happening when single stepping in debuggers
 * and so on. The exception (in my book at least) is for generated code, where you are
 * not maintaining it, but may wish to read and understand it. If you single step it, you know that input()
 * hides some indirect calls, but is always referring to the input stream. This is
 * probably more readable than ctx->input->istream->input(snarfle0->blarg) and allows me to rejig
 * the runtime interfaces without changing the generated code too often, without
 * confusing the reader of the generated output, who may not wish to know the gory
 * details of the interface inheritance.
 */
 
#define		CTX	ctx

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
#undef	    RULEMEMO		    
#undef		DBG

#define	    PARSER							ctx->pParser  
#define	    RECOGNIZER						PARSER->rec
#define		PSRSTATE						RECOGNIZER->state
#define	    HAVEPARSEDRULE(r)				RECOGNIZER->alreadyParsedRule(RECOGNIZER, r)
#define		MEMOIZE(ri,si)					RECOGNIZER->memoize(RECOGNIZER, ri, si)
#define	    INPUT							PARSER->tstream
#define	    STRSTREAM						INPUT
#define		ISTREAM							INPUT->istream
#define		INDEX()							ISTREAM->index(INPUT->istream)
#define	    HASEXCEPTION()					(PSRSTATE->error == ANTLR3_TRUE)
#define	    EXCEPTION						PSRSTATE->exception
#define	    MATCHT(t, fs)					RECOGNIZER->match(RECOGNIZER, t, fs)
#define	    MATCHANYT()						RECOGNIZER->matchAny(RECOGNIZER)
#define	    FOLLOWSTACK					    PSRSTATE->following
#define	    FOLLOWPUSH(x)					FOLLOWSTACK->push(FOLLOWSTACK, ((void *)(&(x))), NULL)
#define	    FOLLOWPOP()						FOLLOWSTACK->pop(FOLLOWSTACK)
#define	    PRECOVER()						RECOGNIZER->recover(RECOGNIZER)
#define	    PREPORTERROR()					RECOGNIZER->reportError(RECOGNIZER)
#define	    LA(n)							INPUT->istream->_LA(ISTREAM, n)
#define	    LT(n)							INPUT->_LT(INPUT, n)
#define	    CONSTRUCTEX()					RECOGNIZER->exConstruct(RECOGNIZER)
#define	    CONSUME()						ISTREAM->consume(ISTREAM)
#define	    MARK()							ISTREAM->mark(ISTREAM)
#define	    REWIND(m)						ISTREAM->rewind(ISTREAM, m)
#define	    REWINDLAST()					ISTREAM->rewindLast(ISTREAM)
#define		SEEK(n)							ISTREAM->seek(ISTREAM, n)
#define	    PERRORRECOVERY					PSRSTATE->errorRecovery
#define	    FAILEDFLAG						PSRSTATE->failed
#define	    HASFAILED()						(FAILEDFLAG == ANTLR3_TRUE)
#define	    BACKTRACKING					PSRSTATE->backtracking
#define	    RECOVERFROMMISMATCHEDSET(s)		RECOGNIZER->recoverFromMismatchedSet(RECOGNIZER, s)
#define	    RECOVERFROMMISMATCHEDELEMENT(e)	RECOGNIZER->recoverFromMismatchedElement(RECOGNIZER, s)
#define     ADAPTOR                         ctx->adaptor
#define		RULEMEMO						PSRSTATE->ruleMemo
#define		DBG								RECOGNIZER->debugger

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
pANTLR3_UINT8   hammerParserTokenNames[8+4]
     = {
        (pANTLR3_UINT8) "<invalid>",       /* String to print to indicate an invalid token */
        (pANTLR3_UINT8) "<EOR>",
        (pANTLR3_UINT8) "<DOWN>", 
        (pANTLR3_UINT8) "<UP>", 
        (pANTLR3_UINT8) "ID",
        (pANTLR3_UINT8) "SLASH",
        (pANTLR3_UINT8) "STRING",
        (pANTLR3_UINT8) "COLON",
        (pANTLR3_UINT8) "STRING_ID",
        (pANTLR3_UINT8) "COMMENT",
        (pANTLR3_UINT8) "WS",
        (pANTLR3_UINT8) "'project'"
       };

        

// Forward declare the locally static matching functions we have generated.
//
static hammerParser_hamfile_return	hamfile    (phammerParser ctx);
static hammerParser_project_def_return	project_def    (phammerParser ctx);
static hammerParser_project_id_return	project_id    (phammerParser ctx);
static void	hammerParserFree(phammerParser ctx);
/* For use in tree output where we are accumulating rule labels via label += ruleRef
 * we need a function that knows how to free a return scope when the list is destroyed. 
 * We cannot just use ANTLR3_FREE because in debug tracking mode, this is a macro.
 */
static	void ANTLR3_CDECL freeScope(void * scope)
{
    ANTLR3_FREE(scope);
}

/** \brief Name of the grammar file that generated this code
 */
static const char fileName[] = "hammer.g";

/** \brief Return the name of the grammar file that generated this code.
 */
static const char * getGrammarFileName()
{
	return fileName;
}
/** \brief Create a new hammerParser parser and return a context for it.
 *
 * \param[in] instream Pointer to an input stream interface.
 *
 * \return Pointer to new parser context upon success.
 */
ANTLR3_API phammerParser
hammerParserNew   (pANTLR3_COMMON_TOKEN_STREAM instream)
{
	// See if we can create a new parser with the standard constructor
	//
	return hammerParserNewSSD(instream, NULL);
}

/** \brief Create a new hammerParser parser and return a context for it.
 *
 * \param[in] instream Pointer to an input stream interface.
 *
 * \return Pointer to new parser context upon success.
 */
ANTLR3_API phammerParser
hammerParserNewSSD   (pANTLR3_COMMON_TOKEN_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state)
{
    phammerParser ctx;	    /* Context structure we will build and return   */
    
    ctx	= (phammerParser) ANTLR3_CALLOC(1, sizeof(hammerParser));
    
    if	(ctx == NULL)
    {
		// Failed to allocate memory for parser context
		//
        return  NULL;
    }
    
    /* -------------------------------------------------------------------
     * Memory for basic structure is allocated, now to fill in
     * the base ANTLR3 structures. We initialize the function pointers
     * for the standard ANTLR3 parser function set, but upon return
     * from here, the programmer may set the pointers to provide custom
     * implementations of each function. 
     *
     * We don't use the macros defined in hammerParser.h here, in order that you can get a sense
     * of what goes where.
     */

    /* Create a base parser/recognizer, using the supplied token stream
     */
    ctx->pParser	    = antlr3ParserNewStream(ANTLR3_SIZE_HINT, instream->tstream, state);
    /* Install the implementation of our hammerParser interface
     */
    ctx->hamfile	= hamfile;
    ctx->project_def	= project_def;
    ctx->project_id	= project_id;
    ctx->free			= hammerParserFree;
    ctx->getGrammarFileName	= getGrammarFileName;
    
    /* Install the scope pushing methods.
     */
    ADAPTOR	= ANTLR3_TREE_ADAPTORNew(instream->tstream->tokenSource->strFactory);
    ctx->vectors	= antlr3VectorFactoryNew(64);
    

	
    /* Install the token table
     */
    PSRSTATE->tokenNames   = hammerParserTokenNames;
    
    
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
	// Free this parser
	//
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

/** Bitset defining follow set for error recovery in rule state: FOLLOW_project_def_in_hamfile56  */
static	ANTLR3_BITWORD FOLLOW_project_def_in_hamfile56_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET_LIST FOLLOW_project_def_in_hamfile56	= { FOLLOW_project_def_in_hamfile56_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_11_in_project_def64  */
static	ANTLR3_BITWORD FOLLOW_11_in_project_def64_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000010) };
static  ANTLR3_BITSET_LIST FOLLOW_11_in_project_def64	= { FOLLOW_11_in_project_def64_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_project_id_in_project_def66  */
static	ANTLR3_BITWORD FOLLOW_project_id_in_project_def66_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET_LIST FOLLOW_project_id_in_project_def66	= { FOLLOW_project_id_in_project_def66_bits, 1	};
/** Bitset defining follow set for error recovery in rule state: FOLLOW_ID_in_project_id73  */
static	ANTLR3_BITWORD FOLLOW_ID_in_project_id73_bits[]	= { ANTLR3_UINT64_LIT(0x0000000000000002) };
static  ANTLR3_BITSET_LIST FOLLOW_ID_in_project_id73	= { FOLLOW_ID_in_project_id73_bits, 1	};
     

 
 
/* ==============================================
 * Parsing rules
 */
/** 
 * $ANTLR start hamfile
 * hammer.g:14:1: hamfile : project_def ;
 */
static hammerParser_hamfile_return
hamfile(phammerParser ctx)
{   
    hammerParser_hamfile_return retval;

    pANTLR3_BASE_TREE root_0;

    hammerParser_project_def_return project_def1;
    #undef	RETURN_TYPE_project_def1
    #define	RETURN_TYPE_project_def1 hammerParser_project_def_return


    /* Initialize rule variables
     */


    root_0 = NULL;

    project_def1.tree = NULL;

    retval.start = LT(1);

    retval.tree  = NULL;
    {
        // hammer.g:14:9: ( project_def )
        // hammer.g:14:11: project_def
        {
            root_0 = (pANTLR3_BASE_TREE)(ADAPTOR->nilNode(ADAPTOR));

            FOLLOWPUSH(FOLLOW_project_def_in_hamfile56);
            project_def1=project_def(ctx);

            FOLLOWPOP();
            if  (HASEXCEPTION())
            {
                goto rulehamfileEx;
            }

            ADAPTOR->addChild(ADAPTOR, root_0, project_def1.tree);

        }

    }
    

    // This is where rules clean up and exit
    //
    goto rulehamfileEx; /* Prevent compiler warnings */
    rulehamfileEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->rulePostProcessing(ADAPTOR, root_0));
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
        retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->errorNode(ADAPTOR, INPUT, retval.start, LT(-1), EXCEPTION));
    }

    return retval;
}
/* $ANTLR end hamfile */

/** 
 * $ANTLR start project_def
 * hammer.g:15:1: project_def : 'project' project_id ;
 */
static hammerParser_project_def_return
project_def(phammerParser ctx)
{   
    hammerParser_project_def_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    string_literal2;
    hammerParser_project_id_return project_id3;
    #undef	RETURN_TYPE_project_id3
    #define	RETURN_TYPE_project_id3 hammerParser_project_id_return

    pANTLR3_BASE_TREE string_literal2_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    string_literal2       = NULL;
    project_id3.tree = NULL;

    retval.start = LT(1);

    string_literal2_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:15:13: ( 'project' project_id )
        // hammer.g:15:15: 'project' project_id
        {
            root_0 = (pANTLR3_BASE_TREE)(ADAPTOR->nilNode(ADAPTOR));

            string_literal2 = (pANTLR3_COMMON_TOKEN) MATCHT(11, &FOLLOW_11_in_project_def64); 
            if  (HASEXCEPTION())
            {
                goto ruleproject_defEx;
            }

            string_literal2_tree = (pANTLR3_BASE_TREE)(ADAPTOR->create(ADAPTOR, string_literal2));
            ADAPTOR->addChild(ADAPTOR, root_0, string_literal2_tree);

            FOLLOWPUSH(FOLLOW_project_id_in_project_def66);
            project_id3=project_id(ctx);

            FOLLOWPOP();
            if  (HASEXCEPTION())
            {
                goto ruleproject_defEx;
            }

            ADAPTOR->addChild(ADAPTOR, root_0, project_id3.tree);

        }

    }
    

    // This is where rules clean up and exit
    //
    goto ruleproject_defEx; /* Prevent compiler warnings */
    ruleproject_defEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->rulePostProcessing(ADAPTOR, root_0));
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
        retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->errorNode(ADAPTOR, INPUT, retval.start, LT(-1), EXCEPTION));
    }

    return retval;
}
/* $ANTLR end project_def */

/** 
 * $ANTLR start project_id
 * hammer.g:16:1: project_id : ID ;
 */
static hammerParser_project_id_return
project_id(phammerParser ctx)
{   
    hammerParser_project_id_return retval;

    pANTLR3_BASE_TREE root_0;

    pANTLR3_COMMON_TOKEN    ID4;

    pANTLR3_BASE_TREE ID4_tree;

    /* Initialize rule variables
     */


    root_0 = NULL;

    ID4       = NULL;
    retval.start = LT(1);

    ID4_tree   = NULL;
    retval.tree  = NULL;
    {
        // hammer.g:16:12: ( ID )
        // hammer.g:16:14: ID
        {
            root_0 = (pANTLR3_BASE_TREE)(ADAPTOR->nilNode(ADAPTOR));

            ID4 = (pANTLR3_COMMON_TOKEN) MATCHT(ID, &FOLLOW_ID_in_project_id73); 
            if  (HASEXCEPTION())
            {
                goto ruleproject_idEx;
            }

            ID4_tree = (pANTLR3_BASE_TREE)(ADAPTOR->create(ADAPTOR, ID4));
            ADAPTOR->addChild(ADAPTOR, root_0, ID4_tree);


        }

    }
    

    // This is where rules clean up and exit
    //
    goto ruleproject_idEx; /* Prevent compiler warnings */
    ruleproject_idEx: ;
    retval.stop = LT(-1);

    	retval.stop = LT(-1);
    	retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->rulePostProcessing(ADAPTOR, root_0));
    	ADAPTOR->setTokenBoundaries(ADAPTOR, retval.tree, retval.start, retval.stop);


    if (HASEXCEPTION())
    {
        PREPORTERROR();
        PRECOVER();
        retval.tree = (pANTLR3_BASE_TREE)(ADAPTOR->errorNode(ADAPTOR, INPUT, retval.start, LT(-1), EXCEPTION));
    }

    return retval;
}
/* $ANTLR end project_id */
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
