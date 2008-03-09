#include "stdafx.h"
#include "pstring.h"
#include "pool.h"
#include <string.h>
#include <iostream>

namespace hammer{

   pstring::pstring(pool& p, const char* first, const char* last)
   {
      s_ = static_cast<char*>(p.malloc(last - first + 1));
      std::copy(first, last, s_);
      size_ = last - first;
      s_[size_] = 0;
   }

   pstring::pstring(pool& p, const char* str)
   {
      size_ = strlen(str);
      s_ = static_cast<char*>(p.malloc(size_ + 1));
      std::copy(str, str + size_ + 1, s_);
   }

   pstring::pstring(pool& p, const std::string& str)
   {
      s_ = static_cast<char*>(p.malloc(str.size() + 1));
      std::copy(str.begin(), str.end(), s_);
      size_ = str.size();
      s_[size_] = 0;
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
      return std::string(s_, s_ + size_);
   }

   bool operator == (const pstring& lhs, const char* rhs)
   {
      return strcmp(lhs.begin(), rhs) == 0;
   }

}