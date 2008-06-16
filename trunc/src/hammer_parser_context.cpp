#include "stdafx.h"
#include "hammer_parser_context.h"
#include "engine.h"
#include "feature.h"
#include "requirements_decl.h"
#include "project_requirements_decl.h"
#include "sources_decl.h"

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
   ctx.current_arg_ = 1; // first argument always project*
}

void on_rule_argument(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ++ctx.current_arg_;
}

bool argument_is_string(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(pstring);
}

bool argument_is_string_list(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(vector<pstring>);
}

bool argument_is_feature(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(hammer::feature);
}
 
bool argument_is_requirements(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(hammer::requirements_decl);
}

bool argument_is_project_requirements(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(hammer::project_requirements_decl);
}

bool argument_is_sources(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.current_rule_->second->args().at(ctx.current_arg_).ti() == typeid(hammer::sources_decl);
}

}}