#pragma once

#include <vector>
#include "pstring.h"
#include "requirements_decl.h"
#include "location.h"

namespace hammer
{
   class feature_set;
   class project;
   class basic_target;
   class main_target;

   class basic_meta_target
   {
      public:
         typedef std::vector<pstring> sources_t;

         basic_meta_target(project* p,
                           const pstring& name, 
                           const requirements_decl& req,
                           const requirements_decl& usage_req);
         basic_meta_target() {}
         virtual ~basic_meta_target();
         const hammer::project* project() const { return project_; }
         const pstring& name() const { return name_; }
         void name(const pstring& v) { name_ = v; }
         void insert(const pstring& source);
         void insert(const std::vector<pstring>& srcs);
         const requirements_decl& usage_requirements() const { return usage_requirements_; }
         const requirements_decl& requirements() const { return requirements_; }
         requirements_decl& usage_requirements() { return usage_requirements_; }
         requirements_decl& requirements() { return requirements_; }
         void usage_requirements(const requirements_decl& r) { usage_requirements_ = r; }
         void requirements(const requirements_decl& r) { requirements_ = r; }
         virtual const location_t& location() const;
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request,
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const = 0;

      protected:
         typedef std::vector<const basic_meta_target*> meta_targets_t;

         void instantiate_simple_targets(const sources_t& targets, 
                                         const feature_set& build_request,
                                         const main_target& owner, 
                                         std::vector<basic_target*>* result) const;
         void instantiate_meta_targets(const meta_targets_t& targets, 
                                       const feature_set& build_request,
                                       const main_target* owner, 
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirments) const;
         void split_sources(sources_t* simple_targets, 
                            meta_targets_t* meta_targets) const;
         void resolve_meta_target_source(const pstring& source, 
                                         sources_t* simple_targets,
                                         meta_targets_t* meta_targets) const;
         // ������������� ��� ��������� alias ������ ��� � �� ����� ��� ��� ����� ������� �����
         virtual void transfer_sources(sources_t* simple_targets, 
                                       meta_targets_t* meta_targets) const;

      private:
         hammer::project* project_;
         pstring name_;
         sources_t sources_;
         requirements_decl requirements_;
         requirements_decl usage_requirements_;
   };
}