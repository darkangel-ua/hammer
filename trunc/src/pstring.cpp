#include "stdafx.h"
#include "pstring.h"
#include "pool.h"
#include <string.h>
#include <iostream>

namespace hammer{

   pstring::pstring(pool& p, const char* first, const char* last)
   {
      s_ = static_cast<char*>(p.malloc(last - first));
      std::copy(first, last, s_);
      size_ = last - first;
   }

   pstring::pstring(pool& p, const char* str)
   {
      size_ = strlen(str);
      s_ = static_cast<char*>(p.malloc(size_));
      std::copy(str, str + size_, s_);
   }

   pstring::pstring(pool& p, const std::string& str)
   {
      s_ = static_cast<char*>(p.malloc(str.size()));
      std::copy(str.begin(), str.end(), s_);
      size_ = str.size();
   }

   std::ostream& operator << (std::ostream& os, const pstring& v)
   {
      os.write(v.s_, v.size_);
      return os;
   }

   bool pstring::operator < (const pstring& rhs) const
   {
      if (size_ == rhs.size_)
         return strncmp(s_, rhs.s_, size_) < 0;
      else
         return size_ < rhs.size_;
   }

   std::string pstring::to_string() const
   {
      return std::string(s_);
   }

}