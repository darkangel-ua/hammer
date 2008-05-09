#pragma once

#include "pstring.h"
#include <vector>

namespace hammer
{
   class project;
   class basic_target;
   class feature_set;
   class type;
   class generated_target;
   class main_target;

   class meta_target
   {
      public:
         typedef std::vector<pstring> sources_t;

         meta_target(project* p, const pstring& name, const feature_set* props,
                     const feature_set* usage_req);
         const pstring& name() const { return name_; }
         void insert(const pstring& source);
         void insert(const std::vector<pstring>& srcs);
         void instantiate(const feature_set& build_request, 
                          std::vector<basic_target*>* result, 
                          feature_set* usage_requirements) const;
         const hammer::project* project() const { return project_; }
         const feature_set& properties() const { return *requirements_; }
         const feature_set& usage_requirements() const { return *usage_requirements_; }

         virtual ~meta_target(){}

      protected:
         typedef std::vector<const meta_target*> meta_targets_t;

         virtual const type* instantiate_type(const feature_set& fs) const = 0;
         void split_sources(sources_t* simple_targets, meta_targets_t* meta_targets) const;

         void instantiate_meta_targets(const meta_targets_t& targets, 
                                       const feature_set& build_request,
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirements) const;
         void instantiate_simple_targets(const sources_t& targets, 
                                         const feature_set& build_request,
                                         const main_target& owner, 
                                         std::vector<basic_target*>* result) const;

      private:
         hammer::project* project_;
         pstring name_;
         sources_t sources_;
         const feature_set* requirements_;
         const feature_set* usage_requirements_;
   };
}
