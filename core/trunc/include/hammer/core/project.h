#if !defined(h_939859ef_4910_49e0_b2b8_fac2a43ebd18)
#define h_939859ef_4910_49e0_b2b8_fac2a43ebd18

#include "location.h"
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <memory>
#include <vector>
#include "basic_meta_target.h"
#include "scm_info.h"
#include "feature_registry.h"
#include "pool.h"

namespace hammer
{
   class feature_set;
   class engine;
   class base_target;

   class project : public basic_meta_target
   {
      public:
         struct selected_target
         {
            selected_target(const basic_meta_target* t, 
                            const feature_set* resolved_build_request,
                            unsigned resolved_build_request_rank)
                           : target_(t),
                             resolved_build_request_(resolved_build_request),
                             resolved_build_request_rank_(resolved_build_request_rank)
            {}
            
            selected_target() 
               : target_(NULL), 
                 resolved_build_request_(NULL) 
            {}

            const basic_meta_target* target_;
            // == project.try_resolve_local_features(build_request)
            const feature_set* resolved_build_request_; 
            unsigned resolved_build_request_rank_;
         };

         typedef boost::ptr_multimap<const pstring /* target name */, basic_meta_target> targets_t;
         typedef std::vector<selected_target> selected_targets_t;

         project(engine* e, 
                 const pstring& name,
                 const location_t& location,
                 const requirements_decl& req,
                 const requirements_decl& usage_req
                 );

         project(engine* e) : engine_(e), 
                              is_root_(false), 
                              add_targets_as_explicit_(false), 
                              local_feature_registry_(&pool_for_feature_registry_) 
         {};
         
         virtual const location_t& location() const { return location_; }
         void location(const location_t& l) { location_ = l; } 
         void scm_info(const hammer::scm_info& info) { scm_info_ = info; }
         const hammer::scm_info& scm_info() const { return scm_info_; }
         hammer::scm_info& scm_info() { return scm_info_; }
         void add_target(std::auto_ptr<basic_meta_target> t);
         const targets_t& targets() const { return targets_; }

         // FIXME: there is can be many metatargets with same name
         const basic_meta_target* find_target(const pstring& name) const;
         // FIXME: there is can be many metatargets with same name
         basic_meta_target* find_target(const pstring& name);

         hammer::engine* get_engine() const { return engine_; }
         const location_t& intermediate_dir() const { return intermediate_dir_; }
         feature_registry& local_feature_registry() const { return local_feature_registry_; }
         bool is_root() const { return is_root_; }
         void set_root(bool v) { is_root_ = v; }
         void add_targets_as_explicit(bool v);
         void mark_as_explicit(const pstring& name);

         void instantiate(const std::string& target_name, 
                          const feature_set& build_request,
                          std::vector<basic_target*>* result) const;
         bool operator == (const project& rhs) const;
         bool operator != (const project& rhs) const { return !(*this == rhs); }
         
         // select targets in project that satisfied build_request. Can return empty list.
         selected_targets_t select_best_alternative(const feature_set& build_request) const;

         // choose best alternative for target_name satisfied build_request
         selected_target select_best_alternative(const pstring& target_name, const feature_set& build_request) const;
         selected_target try_select_best_alternative(const pstring& target_name, const feature_set& build_request) const;
         feature_set* try_resolve_local_features(const feature_set& fs) const;

      private:
         location_t location_;
         targets_t targets_;
         hammer::engine* engine_;
         location_t intermediate_dir_;
         hammer::scm_info scm_info_;
         bool is_root_;
         bool add_targets_as_explicit_;
         pool pool_for_feature_registry_; //FIXME:
         mutable feature_registry local_feature_registry_;

         virtual void instantiate_impl(const main_target* owner, 
                                       const feature_set& build_request,
                                       std::vector<basic_target*>* result, 
                                       feature_set* usage_requirements) const;
   };

   // -1 == not suitable 
   // > -1 == suitable with computed rank. Zero IS valid rank
   int compute_alternative_rank(const feature_set& target_properties, 
                                const feature_set& build_request);

}

#endif //h_939859ef_4910_49e0_b2b8_fac2a43ebd18
