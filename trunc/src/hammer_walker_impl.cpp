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

using namespace std;
using namespace hammer;

void* hammer_make_args_list(void* context)
{
   return new args_list_t();
}

void hammer_rule_call(void* context, const char* rule_name, void* args_list_in)
{
   auto_ptr<args_list_t> args_list(static_cast<args_list_t*>(args_list_in));
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   args_list->insert(args_list->begin(), new call_resolver_call_arg<project>(ctx->project_, false));
   ctx->call_resolver_->invoke(rule_name, *args_list);
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

/*
static type* type_from_file_name(const char* file_name)
{
   return 0;
}      

void hammer_make_project(void* context, const char* project_id)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   auto_ptr<project> p(new project(pstring(ctx->engine_->pstring_pool(), project_id), ctx->location_, ctx->engine_));
   ctx->engine_->insert(p.get());
   ctx->project_ = p.release();
}

static void* hammer_add_meta_target(void* context, const char* target_id, const type& t)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   auto_ptr<meta_target> mt(new meta_target(ctx->project_, 
                                            pstring(ctx->engine_->pstring_pool(), target_id), 
                                            ctx->engine_->get_type_registry().resolve_from_name(t.name())));
   meta_target* result = mt.get();
   ctx->project_->add_target(mt);
   return result;
}

void* hammer_add_lib_meta_target(void* context, const char* target_id)
{
   return hammer_add_meta_target(context, target_id, types::LIB);
}

void* hammer_add_exe_meta_target(void* context, const char* target_id)
{
   return hammer_add_meta_target(context, target_id, types::EXE);
}

void hammer_add_target_to_mt(void* context, void* mt_, const char* target_name_)
{
   hammer_walker_context* ctx = static_cast<hammer_walker_context*>(context);
   meta_target* mt = static_cast<meta_target*>(mt_);
   pstring target_name(ctx->engine_->pstring_pool(), target_name_);
   mt->insert(target_name);
}
*/
