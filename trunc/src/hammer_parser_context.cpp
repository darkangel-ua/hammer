#include "stdafx.h"
#include "hammer_parser_context.h"
#include "engine.h"
#include "feature.h"
#include "requirements_decl.h"
#include "project_requirements_decl.h"
#include "sources_decl.h"
#include "non_buffered_token_stream.h"
#include "feature_registry.h"

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
   ctx.rule_context_.rule_ = i;
   ctx.rule_context_.arg_ = 1; // first argument always project*
}

void on_nested_rule_enter(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.rule_contexts_.push(ctx.rule_context_);
   ctx.rule_context_.arg_ = 1;
   ctx.rule_context_.rule_ = ctx.engine_->call_resolver().end();
}

void on_nested_rule_leave(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.rule_context_ = ctx.rule_contexts_.top();
   ctx.rule_contexts_.pop();
}

void on_rule_argument(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ++ctx.rule_context_.arg_;
}

bool argument_is_string(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(pstring);
}

bool argument_is_string_list(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(vector<pstring>);
}

bool argument_is_feature(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(hammer::feature);
}
 
bool argument_is_requirements(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(hammer::requirements_decl);
}

bool argument_is_project_requirements(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(hammer::project_requirements_decl);
}

bool argument_is_sources(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(hammer::sources_decl);
}

bool is_conditional_feature(pANTLR3_PARSER parser)
{
   pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 4);
   pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 5);
   return t1->stop + 1 == t2->start;
}

void enter_sources_decl(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.token_stream_->ctx_.source_lexing_.push(true);
   ctx.token_stream_->relex_from_current();
}

void leave_sources_decl(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.token_stream_->ctx_.source_lexing_.pop();
   ctx.token_stream_->relex_from_current();
}

void enter_rule_invoke(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.token_stream_->ctx_.source_lexing_.push(false);
   ctx.token_stream_->relex_from_current();
}

void leave_rule_invoke(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.token_stream_->ctx_.source_lexing_.pop();
   ctx.token_stream_->relex_from_current();
}

bool is_path_element(pANTLR3_PARSER parser)
{
	pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1);
	pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2);
	return t1->stop + 1 == t2->start;
}

bool is_path_slash(pANTLR3_PARSER parser)
{
	pANTLR3_COMMON_TOKEN t_1 = parser->tstream->_LT(parser->tstream, -1); // токен до слеша
	pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1); // вот тут находиться проверяемый слеш
	pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2); // токен после слеша
	return t_1->stop + 1 == t1->start || t1->stop + 1 == t2->start;
}

bool is_trailing_slash(pANTLR3_PARSER parser)
{
   pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1); // вот тут находиться проверяемый слеш
   pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2); // токен после слеша
   return t1->stop + 1 != t2->start;
}

bool is_dependency_feature(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   pANTLR3_COMMON_TOKEN feature_name_token = parser->tstream->_LT(parser->tstream, 2);
   pANTLR3_STRING feature_name = feature_name_token->getText(feature_name_token);
   const feature_def* fd = ctx.engine_->feature_registry().find_def(reinterpret_cast<const char*>(feature_name->chars));
   if (fd == NULL)
      return false;
   else
      return fd->attributes().dependency;
}

}}