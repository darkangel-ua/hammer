#include "stdafx.h"
#include "hammer_walker_impl.h"
#include "hammer_walker_context.h"
#include "engine.h"
#include <memory>
#include <vector>
#include "meta_target.h"
#include "type_registry.h"
#include "types.h"

using namespace std;
using namespace hammer;

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
