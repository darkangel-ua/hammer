#pragma once 

#include "location.h"
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <memory>
#include <vector>
#include "basic_meta_target.h"

namespace hammer
{
   class feature_set;
   class engine;
   class base_target;

   class project : public basic_meta_target
   {
      public:
         typedef boost::ptr_multimap<const pstring, basic_meta_target> targets_t;

         project(engine* e, 
                 const pstring& name,
                 const location_t& location,
                 const requirements_decl& req,
                 const requirements_decl& usage_req
                 );

         project(engine* e) : engine_(e) {};
         
         virtual const location_t& location() const { return location_; }
         void location(const location_t& l) { location_ = l; } 
         void add_target(std::auto_ptr<basic_meta_target> t);
         const targets_t& targets() const { return targets_; }
         const basic_meta_target* find_target(const pstring& name) const;
         basic_meta_target* find_target(const pstring& name);
         hammer::engine* engine() const { return engine_; }
         const pstring& intermediate_dir() const { return intermediate_dir_; }

         void instantiate(const std::string& target_name, 
                          const feature_set& build_request,
                          std::vector<basic_target*>* result) const;

      private:
         location_t location_;
         targets_t targets_;
         hammer::engine* engine_;
         pstring intermediate_dir_;

         const basic_meta_target* 
         select_best_alternative(const std::string& target_name, 
                                 const feature_set& f) const;
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request,
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const;
   };
}