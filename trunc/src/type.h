#pragma once
#include <string>

namespace hammer
{
   class type
   {
      public:
         type(const std::string& name, const std::string& suffix, type* base = 0);
         const std::string name() const { return name_; }
         const std::string suffix() const { return suffix_; }
         bool operator == (const type& rhs) const { return name() == rhs.name(); }
         bool operator != (const type& rhs) const { return !(*this == rhs); }

      private:
         std::string name_;
         std::string suffix_;
         type* base_;
   };
}