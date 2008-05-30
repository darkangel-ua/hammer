#include "stdafx.h"
#include "hammer_parser_context.h"
#include "engine.h"
#include "feature.h"
using namespace std;

namespace hammer{ namespace details{

#define MAKE_CTX() hammer_parser_context& ctx = *static_cast<hammer_parser_context*>(parser->super)

void on_enter_rule(pANTLR3_PARSER parser, pANTLR3_UINT8 rule_name_)
{
   MAKE_CTX();
   const char* rule_name = (const char*)rule_name_;

   call_resolver& resolver = ctx.engine_->call_resolver();
   call_resolver::const_iterator i = resolver.find(rule_name);
   if (i == resolver.end())
      throw std::runtime_error("Unknown rule '" + string(rule_name) + "'.");
   ctx.current_rule_ = i;
   ctx.current_arg_ = 0; // first argument always project*
}

void on_rule_argument(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ++ctx.current_arg_;
}

bool argument_is_feature(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(hammer::feature);
}

}}