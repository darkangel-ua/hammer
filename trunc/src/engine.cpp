#include "stdafx.h"
#include "engine.h"
#include "hammer_walker_context.h"
#include "type_registry.h"
#include "types.h"
#include "basic_target.h"
#include "feature_registry.h"
#include "parser.h"

using namespace std;

#undef LIB
#undef EXE

namespace hammer{

   engine::engine(const boost::filesystem::path& root_path) 
      : root_path_(root_path), feature_registry_(0)
   {
      type_registry_.reset(new type_registry);
      auto_ptr<type> cpp(new type(types::CPP));
      type_registry_->insert(cpp);
      auto_ptr<type> h(new type(types::H));
      type_registry_->insert(h);
      auto_ptr<type> lib(new type(types::LIB));
      type_registry_->insert(lib);
      auto_ptr<type> shared_lib(new type(types::SHARED_LIB));
      type_registry_->insert(shared_lib);
      auto_ptr<type> exe(new type(types::EXE));
      type_registry_->insert(exe);

      auto_ptr<hammer::feature_registry> fr(new hammer::feature_registry(&pstring_pool()));

      feature_registry_ = fr.release();
   }

   const project& engine::load_project(const location_t& project_path)
   {
      hammer_walker_context ctx;
      ctx.engine_ = this;
      ctx.location_ = project_path;
      parser p;
      if (!p.parse((root_path_ / project_path / "jamfile").native_file_string().c_str()))
         throw runtime_error("parser errors");
      p.walk(&ctx);

      return *ctx.project_;
   }

   void engine::insert(project* p)
   {
      projects_.insert(p->location(), p);
   }
   
   engine::~engine()
   {
      delete feature_registry_;
   }
}