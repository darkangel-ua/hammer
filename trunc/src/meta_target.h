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
         typedef std::vector<pstring> targets_t;

         meta_target(project* p, const pstring& name, const type* t);
         const pstring& name() const { return name_; }
         void insert(const pstring& t);
         std::vector<basic_target*> instantiate(const feature_set& build_request) const;

      private:
         project* project_;
         pstring name_;
         targets_t targets_;
         const type* type_;
   };
}
