#pragma once
#include <string>
#include <vector>
#include <utility>

namespace hammer
{
   class type;

   class generator
   {
      public:
         typedef std::vector<std::pair<const type*, unsigned int> > types_t;

         generator(const std::string& name,
                   const types_t& source_types,
                   const types_t& target_types);
      
         const std::string& name() const { return name_; }

      private:
         const std::string name_;
         types_t source_types_;
         types_t target_types_;
   };
}