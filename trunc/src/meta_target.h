#pragma once

#include "pstring.h"
#include <vector>

namespace hammer
{
   class project;
   class basic_target;
   class feature_set;
   class type;

   class meta_target
   {
      public:
         typedef std::vector<pstring> sources_t;

         meta_target(project* p, const pstring& name, const type* t);
         const pstring& name() const { return name_; }
         void insert(const pstring& source);
         std::vector<basic_target*> instantiate(const feature_set& build_request) const;
         const hammer::project* project() const { return project_; }

      private:
         hammer::project* project_;
         pstring name_;
         sources_t sources_;
         const type* type_;
   };
}
