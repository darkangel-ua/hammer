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

         meta_target(project* p, const pstring& name, const feature_set* fs);
         const pstring& name() const { return name_; }
         void insert(const pstring& source);
         void insert(const std::vector<pstring>& srcs);
         std::vector<basic_target*> instantiate(const feature_set& build_request) const;
         const hammer::project* project() const { return project_; }
         const feature_set& properties() const { return *features_; }
         virtual ~meta_target(){}

      protected:
         virtual const type* instantiate_type() const = 0;
         std::vector<basic_target*> 
            instantiate_source(main_target* owner, 
                               const pstring& s, 
                               const feature_set& build_request) const;

      private:
         hammer::project* project_;
         pstring name_;
         sources_t sources_;
         const feature_set* features_;
   };
}
