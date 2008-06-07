#include "stdafx.h"
#include "engine.h"
#include "hammer_walker_context.h"
#include "type_registry.h"
#include "types.h"
#include "basic_target.h"
#include "feature_registry.h"
#include "feature_set.h"
#include "feature.h"
#include "parser.h"
#include <boost/bind.hpp>
#include "lib_meta_target.h"
#include "typed_meta_target.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include "generator_registry.h"
#include "msvc_generator.h"
#include "project_requirements_decl.h"

using namespace std;

#undef LIB
#undef EXE

namespace hammer{

engine::engine() 
   :  feature_registry_(0)
{
   type_registry_.reset(new type_registry);
   auto_ptr<type> cpp(new type(types::CPP));
   type_registry_->insert(cpp);
   auto_ptr<type> h(new type(types::H));
   type_registry_->insert(h);
/*
   auto_ptr<type> lib(new type(types::LIB));
   type_registry_->insert(lib);
*/
   auto_ptr<type> static_lib(new type(types::STATIC_LIB));
   type_registry_->insert(static_lib);
   auto_ptr<type> shared_lib(new type(types::SHARED_LIB));
   type_registry_->insert(shared_lib);
   auto_ptr<type> exe(new type(types::EXE));
   type_registry_->insert(exe);
   auto_ptr<type> obj(new type(types::OBJ));
   type_registry_->insert(obj);

   auto_ptr<hammer::feature_registry> fr(new hammer::feature_registry(&pstring_pool()));

   resolver_.insert("project", boost::function<void (project*, vector<pstring>&, project_requirements_decl*, project_requirements_decl*)>(boost::bind(&engine::project_rule, this, _1, _2, _3, _4)));
   resolver_.insert("lib", boost::function<void (project*, vector<pstring>&, vector<pstring>&, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::lib_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("exe", boost::function<void (project*, vector<pstring>&, vector<pstring>&, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::exe_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("import", boost::function<void (vector<pstring>&)>(boost::bind(&engine::import_rule, this, _1)));
   resolver_.insert("feature.feature", boost::function<void (project*, vector<pstring>&, vector<pstring>*, vector<pstring>&)>(boost::bind(&engine::feature_feature_rule, this, _1, _2, _3, _4)));
   resolver_.insert("feature.compose", boost::function<void (project*, feature&, feature_set&)>(boost::bind(&engine::feature_compose_rule, this, _1, _2, _3)));

   {
      feature_attributes ft = {0}; ft.free = 1;
      fr->add_def(feature_def("define", vector<string>(), ft));
   }

   {
      feature_attributes ft = {0}; ft.path = ft.free = 1;
      fr->add_def(feature_def("include", vector<string>(), ft));
   }

   {
      feature_attributes ft = {0}; ft.propagated = 1;
      fr->add_def(feature_def("toolset", vector<string>(), ft));
   }

   {
      feature_attributes ft = {0}; ft.propagated = 1;
      fr->add_def(feature_def("link", boost::assign::list_of<string>("shared")("static"), ft));
   }
   feature_registry_ = fr.release();

   {
      using namespace boost::assign;
      feature_attributes ft = {0};
      ft.propagated = 1;
      vector<string> v;
      v += "debug", "release";
      feature_registry_->add_def(feature_def("variant", v, ft));
   }

   generators_.reset(new generator_registry);
   add_msvc_generators(*this, generators());
}

project* engine::get_upper_project(const location_t& project_path)
{
   location_t upper_path = project_path.branch_path();
   if (exists(upper_path / "jamfile"))
      return &load_project(upper_path);
   
   if (exists(upper_path / "jamroot"))
      return &load_project(upper_path);
   
   return 0;
}

project& engine::load_project(location_t project_path)
{
   projects_t::iterator i = projects_.find(project_path);
   if (i != projects_.end())
      return *i->second;

   hammer_walker_context ctx;
   try
   {
      project_path.normalize();
      ctx.engine_ = this;
      ctx.location_ = project_path;
      ctx.project_ = new project(this);
      ctx.project_->location(pstring(pstring_pool(), project_path.string()));
      ctx.call_resolver_ = &resolver_;

      parser p(this);
      location_t project_file = project_path / "jamfile";
      bool is_top_level = false;
      if (!exists(project_file))
      {
         project_file = project_path / "jamroot";
         is_top_level = true;
      }

      if (!p.parse(project_file.native_file_string().c_str()))
         throw  runtime_error("Can't load project at '"  + project_path.string() + ": parser errors");

      // �� ���� ��� �� ������ ������� ��� ������ ����� ��� ����� ��� ��������� �����������
      // � ��������� �� ��� ����� �������
      if (!is_top_level)
      {
         project* p = get_upper_project(project_path);
         if (p)
         {
            ctx.project_->requirements().insert_infront(p->requirements());
            ctx.project_->usage_requirements().insert_infront(p->usage_requirements());
         }
      }

      p.walk(&ctx);
      assert(ctx.project_);
      insert(ctx.project_);
      return *ctx.project_;
   }
   catch(...)
   {
      delete ctx.project_;
      throw;
   }
}

void engine::insert(project* p)
{
   projects_.insert(location_t(p->location().to_string()), p);
}

engine::~engine()
{
   delete feature_registry_;
}

boost::filesystem::path find_root(const boost::filesystem::path& initial_path)
{
   boost::filesystem::path p(initial_path);
   
   while(true) 
   {
      if (p.empty())
         throw runtime_error("Can't find boost-build.jam");

      if (exists(p / "boost-build.jam"))
         return p;
      
      p = p.branch_path();
   };
}

void engine::project_rule(project* p, std::vector<pstring>& name,
                          project_requirements_decl* req, project_requirements_decl* usage_req)
{
   assert(name.size() == size_t(1));
   p->name(name[0]);
   
   // ����� ������������� ������� ��� ��� �� ����� �������� 
   // engine ��� ��������� �������������� �������� ������� �� ��� ��������
   if (req)
      p->requirements().insert(req->requirements());
   if (usage_req)
      p->usage_requirements().insert(usage_req->requirements());
}

void engine::lib_rule(project* p, std::vector<pstring>& name, std::vector<pstring>& sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<meta_target> mt(new lib_meta_target(p, name.at(0), requirements ? *requirements : requirements_decl(), 
                                                usage_requirements ? *usage_requirements : requirements_decl()));
   mt->insert(sources);
   p->add_target(mt);
}

void engine::exe_rule(project* p, std::vector<pstring>& name, std::vector<pstring>& sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<meta_target> mt(new typed_meta_target(p, name.at(0), requirements ? *requirements : requirements_decl(), 
                                                  usage_requirements ? *usage_requirements : requirements_decl(), 
                                                  get_type_registry().resolve_from_name(types::EXE.name())));
   mt->insert(sources);
   p->add_target(mt);
}

void engine::import_rule(std::vector<pstring>& name)
{

}

static feature_attributes resolve_attributes(std::vector<pstring>& attributes)
{
   typedef std::vector<pstring>::const_iterator iter;
   feature_attributes result = {0};
   
   iter i = find(attributes.begin(), attributes.end(), "propagated");
   if (i != attributes.end())
      result.propagated = true;

   i = find(attributes.begin(), attributes.end(), "composite");
   if (i != attributes.end())
      result.composite = true;

   i = find(attributes.begin(), attributes.end(), "free");
   if (i != attributes.end())
      result.free = true;

   i = find(attributes.begin(), attributes.end(), "path");
   if (i != attributes.end())
      result.path = true;
   
   i = find(attributes.begin(), attributes.end(), "incidental");
   if (i != attributes.end())
      result.incidental = true;

   i = find(attributes.begin(), attributes.end(), "optional");
   if (i != attributes.end())
      result.optional = true;

   i = find(attributes.begin(), attributes.end(), "symmetric");
   if (i != attributes.end())
      result.symmetric = true;

   return result;
}

void engine::feature_feature_rule(project* p, std::vector<pstring>& name, 
                                  std::vector<pstring>* values,
                                  std::vector<pstring>& attributes)
{
   if (name.empty() || name.size() > 1)
      throw std::runtime_error("[feature.feature] Bad feature name.");

   vector<string> def_values;
   if (values)
   {
      for(vector<pstring>::const_iterator i = values->begin(), last = values->end(); i != last; ++i)
         def_values.push_back(i->to_string());
   }

   feature_def def(name[0].to_string(), def_values, resolve_attributes(attributes));

   feature_registry_->add_def(def);
}

void engine::feature_compose_rule(project* p, feature& f, feature_set& components)
{
   feature_set* cc = components.clone();
   feature_registry_->get_def(f.def().name()).compose(f.value().to_string(), cc);
}

}
