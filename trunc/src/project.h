#pragma once 

#include "location.h"
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <memory>
#include <vector>
#include "meta_target.h"
#include "basic_meta_target.h"

namespace hammer
{
   class feature_set;
   class engine;
   class base_target;

   class project : public basic_meta_target
   {
      public:
         typedef boost::ptr_multimap<const pstring, meta_target> targets_t;

         project(engine* e, 
                 const pstring& name,
                 const pstring& location,
                 const requirements_decl& req,
                 const requirements_decl& usage_req
                 );

         project(engine* e) : engine_(e) {};
         
         virtual const pstring& location() const { return location_; }
         void location(const pstring& l) { location_ = l; } 
         void add_target(std::auto_ptr<meta_target> t);
         const targets_t& targets() const { return targets_; }
         const meta_target* find_target(const pstring& name) const;
         hammer::engine* engine() const { return engine_; }
         const pstring& intermediate_dir() const { return intermediate_dir_; }

         void instantiate(const std::string& target_name, 
                          const feature_set& build_request,
                          std::vector<basic_target*>* result) const;

      private:
         pstring location_;
         targets_t targets_;
         hammer::engine* engine_;
         pstring intermediate_dir_;

         const meta_target* 
         select_best_alternative(const std::string& target_name, 
                                 const feature_set& f) const;
   };
}