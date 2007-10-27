#pragma once 

#include "location.h"
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <memory>
#include <vector>
#include "meta_target.h"

namespace hammer
{
   class feature_set;
   class engine;
   class base_target;

   class project
   {
      public:
         typedef boost::ptr_multimap<const pstring, meta_target> targets_t;
         project(const pstring& id,
                 const location_t& location,
                 engine* e);
         
         const location_t& location() const { return location_; }
         const pstring& id() const { return id_; }
         void add_target(std::auto_ptr<meta_target> t);
         const targets_t& targets() const { return targets_; }
         const meta_target* find_target(const pstring& name) const;
         hammer::engine* engine() const { return engine_; }

         std::vector<basic_target*> instantiate(const std::string& target_name, const feature_set& build_request) const;

      private:
         pstring id_;
         location_t location_;
         targets_t targets_;
         hammer::engine* engine_;
         const meta_target* 
         select_best_alternative(const std::string& target_name, 
                                 const feature_set& f) const;
   };
}