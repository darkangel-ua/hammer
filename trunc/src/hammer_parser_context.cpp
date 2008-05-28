#include "stdafx.h"
#include "hammer_parser_context.h"

namespace hammer{ namespace details{

#define MAKE_CTX() hammer_parser_context& ctx = *static_cast<hammer_parser_context*>(parser->super)

void on_rule(pANTLR3_PARSER parser)
{
   MAKE_CTX();
}

}}