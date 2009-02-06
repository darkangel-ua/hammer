#include "stdafx.h"
#include "hammer_lexer_context.h"

namespace hammer{namespace details{

bool is_source_lexing(pANTLR3_LEXER lexer)
{
   hammer_lexer_context* ctx = static_cast<hammer_lexer_context*>(lexer->super);
   if (ctx->source_lexing_.empty())
      return false;
   
   return ctx->source_lexing_.top();
}

}}