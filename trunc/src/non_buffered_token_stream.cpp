#include "stdafx.h"
#include "non_buffered_token_stream.h"

namespace hammer{ namespace details{

static void fillBuffer(pANTLR3_COMMON_TOKEN_STREAM tokenStream, int k, bool with_skiping);

static void nbs_free(struct ANTLR3_COMMON_TOKEN_STREAM_struct * tokenStream)
{
   delete static_cast<non_buffered_token_stream*>(tokenStream->super);
}

static ANTLR3_UINT32
skipOffTokenChannelsReverse(pANTLR3_COMMON_TOKEN_STREAM tokenStream, ANTLR3_INT32 x)
{
   pANTLR3_COMMON_TOKEN    tok;

   while   ( x >= 0 )
   {
      tok =	tokenStream->tstream->get(tokenStream->tstream, x);

      if  (tok == NULL || (tok->getChannel(tok) != tokenStream->channel))
      {
         x--;
      }
      else
      {
         return x;
      }
   }
   return x;
}

static	pANTLR3_COMMON_TOKEN
LB  (pANTLR3_COMMON_TOKEN_STREAM cts, ANTLR3_INT32 k)
{
   ANTLR3_INT32    i;
   ANTLR3_INT32    n;

   if	(cts->p == -1)
   {
      fillBuffer(cts, k, true);
   }
   if	(k == 0)
   {
      return NULL;
   }
   if	((cts->p - k) < 0)
   {
      return NULL;
   }

   i	= cts->p;
   n	= 1;

   /* Need to find k good tokens, going backwards, skipping ones that are off channel
   */
   while   (  n <= (ANTLR3_INT32)k )
   {
      /* Skip off-channel tokens 
      */

      i = skipOffTokenChannelsReverse(cts, i-1); /* leave p on valid token    */
      n++;
   }
   if	( i <0 )
   {
      return	NULL;
   }

   return  (pANTLR3_COMMON_TOKEN)cts->tokens->get(cts->tokens, i);
}

static ANTLR3_UINT32
skipOffTokenChannels(pANTLR3_COMMON_TOKEN_STREAM tokenStream, ANTLR3_INT32 i)
{
   ANTLR3_INT32	    n;
   pANTLR3_COMMON_TOKEN    tok;

   n	= tokenStream->tstream->istream->cachedSize;

   while   (	   i < n )
   {
      tok =	tokenStream->tstream->get(tokenStream->tstream, i);

      if  (tok == NULL || tok->getChannel(tok) != tokenStream->channel)
      {
         i++;
         if (i == n)
         fillBuffer(tokenStream, 1, false);
         n	= tokenStream->tstream->istream->cachedSize;
      }
      else
      {
         return i;
      }
   }
   return i;
}

static void fillBuffer(pANTLR3_COMMON_TOKEN_STREAM tokenStream, int k, bool with_skiping)
{
   ANTLR3_UINT32	    index;
   pANTLR3_COMMON_TOKEN    tok;
   ANTLR3_BOOLEAN	    discard;
   void		  * channelI;

   /* Start at index 0 of course
   */
   index   = 0;

   /* Pick out the next token from the token source
   * Remember we just get a pointer (reference if you like) here
   * and so if we store it anywhere, we don't set any pointers to auto free it.
   */
   tok	    = tokenStream->tstream->tokenSource->nextToken(tokenStream->tstream->tokenSource);

   while   (tok != NULL && tok->type != ANTLR3_TOKEN_EOF)
   {
      discard	    = ANTLR3_FALSE;	/* Assume we are not discarding	*/

      /* I employ a bit of a trick, or perhaps hack here. Rather than
      * store a pointer to a structure in the override map and discard set
      * we store the value + 1 cast to a void *. Hence on systems where NULL = (void *)0
      * we can distinguish "not being there" from "being channel or type 0"
      */

      if  (	tokenStream->discardSet							    != NULL
         && tokenStream->discardSet->get(tokenStream->discardSet, tok->getType(tok))    != NULL)
      {
         discard = ANTLR3_TRUE;
      }
      else if (      tokenStream->discardOffChannel	== ANTLR3_TRUE
         && tok->getChannel(tok)		!= tokenStream->channel
         )
      {
         discard = ANTLR3_TRUE;
      }
      else if	(   tokenStream->channelOverrides != NULL)
      {
         /* See if this type is in the override map
         */
         channelI	= tokenStream->channelOverrides->get(tokenStream->channelOverrides, tok->getType(tok)+1);

         if	(channelI != NULL)
         {
            /* Override found
            */
            tok->setChannel(tok, ANTLR3_UINT32_CAST(channelI) - 1);
         }
      }

      /* If not discarding it, add it to the list at the current index
      */
      if  (discard == ANTLR3_FALSE)
      {
         /* Add it, indicating that we will delete it and the table should not
         */
         tok->setTokenIndex(tok, index);
         tokenStream->tokens->add(tokenStream->tokens, (void *)tok, NULL);
         index++;
         if (--k == 0)
            break;
      }

      tok = tokenStream->tstream->tokenSource->nextToken(tokenStream->tstream->tokenSource);
   }

   /* Set the consume pointer to the first token that is on our channel
   */
   if (tokenStream->p == -1)
      tokenStream->p = 0;

   if (with_skiping)
      tokenStream->p  = skipOffTokenChannels(tokenStream, tokenStream->p);

   /* Cache the size so we don't keep doing indirect method calls
   */
   tokenStream->tstream->istream->cachedSize = tokenStream->tokens->count;
}

static pANTLR3_COMMON_TOKEN nbs_tokLT(pANTLR3_TOKEN_STREAM ts, ANTLR3_INT32 k)
{
   ANTLR3_INT32    i;
   ANTLR3_INT32    n;
   pANTLR3_COMMON_TOKEN_STREAM cts;

   cts	    = (pANTLR3_COMMON_TOKEN_STREAM)ts->super;


   if	(k == 0)
      return NULL;

   if	(cts->p == -1 ||
       (cts->p + k - 1) >= (ANTLR3_INT32)ts->istream->cachedSize)
   {
      fillBuffer(cts, k, true);
   }

   if	(k < 0)
      return LB(cts, -k);

   if	((cts->p + k - 1) >= (ANTLR3_INT32)ts->istream->cachedSize)
   {
      pANTLR3_COMMON_TOKEN    teof = &(ts->tokenSource->eofToken);

      teof->setStartIndex (teof, ts->istream->index	    (ts->istream));
      teof->setStopIndex  (teof, ts->istream->index	    (ts->istream));
      return  teof;
   }

   i	= cts->p;
   n	= 1;

   /* Need to find k good tokens, skipping ones that are off channel
   */
   while   ( n < k)
   {
      /* Skip off-channel tokens */
      i = skipOffTokenChannels(cts, i+1); /* leave p on valid token    */
      n++;
   }
   if	( (ANTLR3_UINT32) i > ts->istream->cachedSize)
   {
      pANTLR3_COMMON_TOKEN    teof = &(ts->tokenSource->eofToken);

      teof->setStartIndex (teof, ts->istream->index(ts->istream));
      teof->setStopIndex  (teof, ts->istream->index(ts->istream));
      return  teof;
   }

   return  (pANTLR3_COMMON_TOKEN)cts->tokens->get(cts->tokens, i);
}

pANTLR3_COMMON_TOKEN_STREAM non_buffered_token_stream::create(ANTLR3_UINT32 hint, pANTLR3_TOKEN_SOURCE source)
{
   non_buffered_token_stream* nbs = new non_buffered_token_stream;
   pANTLR3_COMMON_TOKEN_STREAM tstream_ = antlr3CommonTokenStreamSourceNew(hint, source);
   tstream_->super = nbs;
   nbs->base_free = tstream_->free;
   tstream_->free = nbs_free;
   tstream_->tstream->_LT = nbs_tokLT;
   
   return tstream_;
}

bool is_lexing_sources_decl(pANTLR3_LEXER lexer)
{
   hammer_lexer_context* ctx = static_cast<hammer_lexer_context*>(lexer->super);
   if (ctx->source_lexing_.empty())
      return false;
   
   return ctx->source_lexing_.top();
}

}}