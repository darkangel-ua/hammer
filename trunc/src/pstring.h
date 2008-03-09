#pragma once

#include <iosfwd>
#include <string>

namespace hammer
{
   class pool;
   class pstring
   {
      public:
         typedef unsigned int size_type;
         pstring(pool& p, const char* first, const char* last);
         pstring(pool& p, const char* str);
         pstring(pool& p, const std::string& str);
         size_type size() const { return size_; }
         std::string to_string() const;
         const char* begin() const { return s_; }

         ~pstring(){};
         friend std::ostream& operator << (std::ostream& os, const pstring& rhs);
         bool operator < (const pstring& rhs) const;

      private:
         char* s_;
         size_type size_;
   };

   bool operator == (const pstring& lhs, const char* rhs);
}