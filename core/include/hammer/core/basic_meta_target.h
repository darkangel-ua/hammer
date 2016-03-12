#if !defined(h_28ffa573_8792_4330_8c82_27921827e424)
#define h_28ffa573_8792_4330_8c82_27921827e424

#include <vector>
#include "pstring.h"
#include "requirements_decl.h"
#include "location.h"
#include "sources_decl.h"

namespace hammer
{
   class feature_set;
   class project;
   class engine;
   class basic_target;
   class main_target;
   class type_registry;

   class basic_meta_target
   {
      public:
         basic_meta_target(project* p,
                           const pstring& name, 
                           const requirements_decl& req,
                           const requirements_decl& usage_req);
         basic_meta_target() {}
         virtual ~basic_meta_target();
         const project* get_project() const { return project_; }
         engine* get_engine() const;
         const pstring& name() const { return name_; }
         void name(const pstring& v) { name_ = v; }
         void sources(const sources_decl& s);
         void add_sources(const sources_decl& s);
         const sources_decl& sources() const { return sources_; }
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
                                  feature_set* usage_requirements) const;

         void set_explicit(bool v) { is_explicit_ = v; }
         bool is_explicit() const { return is_explicit_; }

      protected:
         typedef std::vector<std::pair<const basic_meta_target* /* target */, 
                                       const feature_set* /* requested build properties*/> > meta_targets_t;

         // returns true if instantiate() can cache instantiation for build_request
         virtual bool is_cachable(const main_target* owner) const { return true; }

         virtual void instantiate_impl(const main_target* owner, 
                                       const feature_set& build_request,
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirements) const = 0;
         
         void instantiate_simple_targets(const sources_decl& targets, 
                                         const feature_set& build_request,
                                         const main_target& owner, 
                                         std::vector<basic_target*>* result) const;
         void instantiate_meta_targets(const meta_targets_t& targets, 
                                       const feature_set& build_request,
                                       const main_target* owner, 
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirments) const;
         void split_sources(sources_decl* simple_targets, 
                            meta_targets_t* meta_targets,
                            const sources_decl& sources, 
                            const feature_set& build_request) const;
         void split_one_source(sources_decl* simple_targets,
                               meta_targets_t* meta_targets,
                               const source_decl& source,
                               const feature_set& build_request,
                               const type_registry& tr) const;
         void resolve_meta_target_source(const source_decl& source,
                                         const feature_set& build_request, 
                                         sources_decl* simple_targets,
                                         meta_targets_t* meta_targets) const;
         // this is introduced to support alias meta target, but lately was added more logic
         // possible FIXME: rename to reflect real functionality
         virtual void transfer_sources(sources_decl* simple_targets, 
                                       meta_targets_t* meta_targets,
                                       const feature_set& build_request,
                                       const feature_set* additional_build_properties,
                                       const basic_meta_target* transfer_target) const;

         const feature_set& resolve_undefined_features(const feature_set& fs) const;

      private:
         struct cached_instantiation_data_t
         {
            const feature_set* build_request_;
            std::vector<basic_target*> instantiated_targets_;
            feature_set* computed_usage_requirements_;
         };
         typedef std::vector<cached_instantiation_data_t> instantiation_cache_t;

         hammer::project* project_;
         pstring name_;
         sources_decl sources_;
         requirements_decl requirements_;
         requirements_decl usage_requirements_;
         bool is_explicit_;
         mutable instantiation_cache_t instantiation_cache_;
   };
}

#endif //h_28ffa573_8792_4330_8c82_27921827e424
