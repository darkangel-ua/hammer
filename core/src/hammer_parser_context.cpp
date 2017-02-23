#include "stdafx.h"
#include "hammer_parser_context.h"
#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/project_requirements_decl.h>
#include <hammer/core/sources_decl.h>
#include "non_buffered_token_stream.h"
#include <hammer/core/feature_registry.h>

using namespace std;

namespace hammer{ namespace details{

#define MAKE_CTX() hammer_parser_context& ctx = *static_cast<hammer_parser_context*>(parser->super)

void on_enter_rule(pANTLR3_PARSER parser, pANTLR3_UINT8 rule_name_)
{
   MAKE_CTX();
   const char* rule_name = reinterpret_cast<const char*>(rule_name_);
   call_resolver& resolver = ctx.engine_->call_resolver();
   call_resolver::const_iterator i = resolver.find(rule_name);
   if (i == resolver.end())
      throw std::runtime_error("Unknown rule '" + string(rule_name) + "'.");
   
   // this block implement feature creation hook to be able to answer on is_dependency_feature 
   // question later, when feature will appear in code
   if (strcmp("feature.feature", rule_name) == 0)
   {
      pANTLR3_COMMON_TOKEN feature_name_token = parser->tstream->_LT(parser->tstream, 1);
      if (feature_name_token != &parser->tstream->tokenSource->eofToken)
      {
         const char* feature_name = reinterpret_cast<const char*>(feature_name_token->getText(feature_name_token)->chars);
         if (feature_name != NULL)
         {
            ctx.rule_context_.in_feature_feature_rule_ = true;
            typedef hammer_parser_context::new_features_t::iterator iter;
            iter i = ctx.new_features_.insert({feature_name, {feature_name, false}}).first;
            ctx.rule_context_.new_feature_ = &i->second;
         }
      }
   }

   ctx.rule_context_.rule_ = i;
   ctx.rule_context_.arg_ = 1; // first argument always project*
}

void on_nested_rule_enter(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.rule_contexts_.push(ctx.rule_context_);
   ctx.rule_context_.arg_ = 1;
   ctx.rule_context_.rule_ = ctx.engine_->call_resolver().end();

   // disable source lexing if any
   ctx.token_stream_->ctx_.source_lexing_.push(false);
   ctx.token_stream_->relex_from_current();
}

void on_nested_rule_leave(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ctx.rule_context_ = ctx.rule_contexts_.top();
   ctx.rule_contexts_.pop();

   // enable source lexing if any
   ctx.token_stream_->ctx_.source_lexing_.pop();
   ctx.token_stream_->relex_from_current();
}

void on_rule_argument(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   ++ctx.rule_context_.arg_;
}

void on_string_list_element(pANTLR3_PARSER parser, pANTLR3_UINT8 id_)
{
   MAKE_CTX();
   
   // this is feature creation hook
   const char* id = reinterpret_cast<const char*>(id_);
   if (ctx.rule_context_.in_feature_feature_rule_ &&
       ctx.rule_context_.arg_ == 3 &&
       strcmp(id, "dependency") == 0)
   {
      ctx.rule_context_.new_feature_->dependency_ = true;
   }
}

bool argument_is_string(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(string);
}

bool argument_is_string_list(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   return ctx.rule_context_.rule_->second->args().at(ctx.rule_context_.arg_).ti() == typeid(vector<string>);
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
   pANTLR3_COMMON_TOKEN t_1 = parser->tstream->_LT(parser->tstream, -1);
   pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1);
   pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2);
   return t_1->stop + 1 == t1->start && t1->stop + 1 == t2->start;
}

bool is_path_slash(pANTLR3_PARSER parser)
{
	pANTLR3_COMMON_TOKEN t_1 = parser->tstream->_LT(parser->tstream, -1);
	pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1);
	pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2);
	return t_1->stop + 1 == t1->start && t1->stop + 1 == t2->start;
}

bool is_trailing_slash(pANTLR3_PARSER parser)
{
   pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1);
   pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2);
   return t1->stop + 1 != t2->start;
}

bool is_head_slash(pANTLR3_PARSER parser)
{
   pANTLR3_COMMON_TOKEN t1 = parser->tstream->_LT(parser->tstream, 1);
   pANTLR3_COMMON_TOKEN t2 = parser->tstream->_LT(parser->tstream, 2);
   return t1->stop + 1 == t2->start;
}

bool is_dependency_feature(pANTLR3_PARSER parser)
{
   MAKE_CTX();
   pANTLR3_COMMON_TOKEN feature_name_token = parser->tstream->_LT(parser->tstream, 2);
   const char* feature_name = reinterpret_cast<const char*>(feature_name_token->getText(feature_name_token)->chars);
   const feature_def* fd = ctx.engine_->feature_registry().find_def(feature_name);
   if (fd == NULL)
   {
      hammer_parser_context::new_features_t::const_iterator i = ctx.new_features_.find(feature_name);
      if (i != ctx.new_features_.end())
         return i->second.dependency_;
      else
         return false;
   }
   else
      return fd->attributes().dependency;
}

}}
