#include "stdafx.h"
#include "hammer_walker_impl.h"
#include "hammer_walker_context.h"
#include "engine.h"
#include <memory>
#include <vector>
#include "meta_target.h"
#include "type_registry.h"
#include "types.h"
#include "call_resolver.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"
#include "project_requirements_decl.h"
#include "sources_decl.h"
#include <antlr3commontoken.h>
#include <antlr3input.h>

using namespace std;
using namespace hammer;
namespace fs = boost::filesystem;

void* hammer_make_args_list(void* context)
{
   return new args_list_t();
}

void* hammer_rule_call(void* context, const char* rule_name, void* args_list_in)
{
   auto_ptr<args_list_t> args_list(static_cast<args_list_t*>(args_list_in));
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   args_list->insert(args_list->begin(), new call_resolver_call_arg<project>(ctx->project_, false));
   auto_ptr<call_resolver_call_arg_base> result(ctx->call_resolver_->invoke(rule_name, *args_list));
   return result.release();
}

void hammer_delete_rule_result(void* result)
{
   delete static_cast<call_resolver_call_arg_base*>(result);
}

void* hammer_make_null_arg()
{
   return new call_resolver_call_arg<null_arg>(0, false);
}

void* hammer_make_string_list()
{
   return new call_resolver_call_arg<std::vector<pstring> >(new std::vector<pstring>(), true);
}

void* hammer_make_feature_list(void* context)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   return new call_resolver_call_arg<feature_set>(new feature_set(&ctx->engine_->feature_registry()), false);
}

void* hammer_make_requirements_decl()
{
   return new requirements_decl;
}

void hammer_add_arg_to_args_list(void* args_list, void* arg)
{
   args_list_t* args_list_ = static_cast<args_list_t*>(args_list);
   call_resolver_call_arg_base* arg_ = static_cast<call_resolver_call_arg_base*>(arg);
   args_list_->push_back(arg_);
}

void hammer_add_id_to_string_list(void* context, void* string_list, const char* id)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   call_resolver_call_arg<std::vector<pstring> >* string_list_ = static_cast<call_resolver_call_arg<std::vector<pstring> >*>(string_list);
   string_list_->value()->push_back(pstring(ctx->engine_->pstring_pool(), id));
}

void hammer_add_feature_to_list(void* context, void* args_list, const char* feature_name, const char* feature_value)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   call_resolver_call_arg<feature_set>* args_list_ = static_cast<call_resolver_call_arg<feature_set>*>(args_list);
   args_list_->value()->join(feature_name, feature_value);
}

void hammer_add_feature_argument(void* context, void* args_list, const char* feature_name, const char* feature_value)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   args_list_t* args_list_ = static_cast<args_list_t*>(args_list);
   call_resolver_call_arg<feature>* arg = new call_resolver_call_arg<feature>(ctx->engine_->feature_registry().create_feature(feature_name, feature_value), false);
   args_list_->push_back(arg);
}

void hammer_add_string_arg_to_args_list(void* context, void* args_list, const char* id)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   call_resolver_call_arg<pstring>* arg = new call_resolver_call_arg<pstring>(new pstring(ctx->engine_->pstring_pool(), id), false);
   args_list_t* args_list_ = static_cast<args_list_t*>(args_list);
   args_list_->push_back(arg);
}

void* hammer_create_feature(void* context, const char* feature_name, const char* feature_value)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   return ctx->engine_->feature_registry().create_feature(feature_name, feature_value == NULL ? "" : feature_value);
}

void* hammer_make_requirements_condition()
{
   return new linear_and_condition;
}

void* hammer_make_project_requirements_decl(const char* id, void* rdecl)
{
   requirements_decl* r = static_cast<requirements_decl*>(rdecl);
   project_requirements_decl* result = new project_requirements_decl(id, *r);
   delete r;
   return result;
}

void* hammer_make_requirements_decl_arg(void* rdecl)
{
   requirements_decl* r = static_cast<requirements_decl*>(rdecl);
   return new call_resolver_call_arg<requirements_decl>(r, true);
}

void* hammer_make_project_requirements_decl_arg(void* pdecl)
{
   project_requirements_decl* p = static_cast<project_requirements_decl*>(pdecl);
   return new call_resolver_call_arg<project_requirements_decl>(p, true);
}

void hammer_add_conditional_to_rdecl(void* condition, void* rdecl)
{
   requirements_decl* r = static_cast<requirements_decl*>(rdecl);
   std::auto_ptr<requirement_base> c(static_cast<linear_and_condition*>(condition)); 
   r->add(c);
}

void hammer_add_feature_to_rdecl(void* feature, void* rdecl)
{
   requirements_decl* r = static_cast<requirements_decl*>(rdecl);
   std::auto_ptr<requirement_base> nr(new just_feature_requirement(static_cast<hammer::feature*>(feature)));
   r->add(nr);
}

void hammer_set_condition_result(void* condition, void* feature)
{
   linear_and_condition* c = static_cast<linear_and_condition*>(condition);
   c->result(static_cast<hammer::feature*>(feature));
}

void hammer_add_feature_to_condition(void* feature, void* condition)
{
   linear_and_condition* c = static_cast<linear_and_condition*>(condition);
   c->add(static_cast<hammer::feature*>(feature));
}

void* hammer_make_sources_decl()
{
   return new sources_decl();
}

void hammer_add_source_to_sources_decl(void* result, void* sd)
{
   source_decl* source_decl = static_cast<hammer::source_decl*>(sd);
   static_cast<sources_decl*>(result)->push_back(*source_decl);
   delete source_decl;
}

void* hammer_make_sources_decl_arg(void* s)
{
   return new call_resolver_call_arg<sources_decl>(s, true);
}

void hammer_add_rule_result_to_source_decl(void* rule_result, void* sources)
{
   call_resolver_call_arg<sources_decl>* rr = static_cast<call_resolver_call_arg<sources_decl>*>(rule_result);
   static_cast<sources_decl*>(sources)->transfer_from(*rr->value());
}

void* hammer_make_path()
{
   return new fs::path();
}

void hammer_add_to_path(void* p, const char* token)
{
   fs::path* pd = static_cast<fs::path*>(p);
   *pd /= token;
}

void* hammer_make_path_arg(void* p)
{
   call_resolver_call_arg<fs::path>* result = new call_resolver_call_arg<fs::path>(static_cast<fs::path*>(p), true);
   return result;
}

void* hammer_make_feature_set_arg(void* fs)
{
   return new call_resolver_call_arg<feature_set>(fs, false);
}

void* hammer_make_feature_arg(void* f)
{
   return new call_resolver_call_arg<feature>(f, false);
}

void* hammer_make_source_decl()
{
   return new source_decl;
}

typedef std::pair<pANTLR3_COMMON_TOKEN, pANTLR3_COMMON_TOKEN> target_path_t;

void hammer_source_decl_set_target_path(void* context, void* sd, void* tp)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   source_decl* source_decl = static_cast<hammer::source_decl*>(sd);
   target_path_t* target_path_tokens = static_cast<target_path_t*>(tp);

   if (target_path_tokens->second == NULL)
      target_path_tokens->second = target_path_tokens->first;

   pANTLR3_STRING s = target_path_tokens->first->input->substr(target_path_tokens->first->input, 
                                                               target_path_tokens->first->start, 
                                                               target_path_tokens->second->stop);
   pstring target_path(ctx->engine_->pstring_pool(), reinterpret_cast<const char*>(s->chars));
   source_decl->target_path(target_path, ctx->engine_->get_type_registry().resolve_from_target_name(target_path));
   
   delete target_path_tokens;
   s->factory->destroy(s->factory, s);
}

void hammer_source_decl_set_target_name(void* context, void* sd, const char* id)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   source_decl* source_decl = static_cast<hammer::source_decl*>(sd);

   if (id != NULL)
      source_decl->target_name(pstring(ctx->engine_->pstring_pool(), id));
}

void hammer_source_decl_set_target_properties(void* sd, void* fs)
{
   source_decl* source_decl = static_cast<hammer::source_decl*>(sd);
   source_decl->properties(static_cast<hammer::feature_set*>(fs));
}

void hammer_source_decl_set_public(void* sd)
{
   source_decl* source_decl = static_cast<hammer::source_decl*>(sd);
   source_decl->set_public(true);
}

void* hammer_make_target_path()
{
   return new target_path_t();
}

void hammer_add_to_target_path(void* context, void* tp, pANTLR3_BASE_TREE node)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   target_path_t* target_path = static_cast<target_path_t*>(tp);

   if (target_path->first == NULL)
      target_path->first = node->getToken(node);
   else
      target_path->second = node->getToken(node);
}

void* hammer_make_feature_set(void* context)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   return ctx->engine_->feature_registry().make_set();
}

void hammer_add_feature_to_feature_set(void* feature_set, void* feature)
{
   static_cast<hammer::feature_set*>(feature_set)->join(static_cast<hammer::feature*>(feature));   
}

void hammer_feature_set_dependency_data(void* f, void* sd_)
{
   source_decl* sd = static_cast<source_decl*>(sd_);
   static_cast<feature*>(f)->get_dependency_data().source_ = *sd;
   delete sd;
}
