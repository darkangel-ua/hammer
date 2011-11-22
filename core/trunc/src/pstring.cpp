#include "stdafx.h"
#include <hammer/core/pstring.h>
#include <hammer/core/pool.h>
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
      if (str == NULL)
      {
         size_ = 0;
         s_ = "";
      }
      else
      {
         size_ = strlen(str);
         s_ = static_cast<char*>(p.malloc(size_ + 1));
         std::copy(str, str + size_ + 1, s_);
      }
   }

   pstring::pstring(pool& p, const std::string& str)
   {
      s_ = static_cast<char*>(p.malloc(str.size() + 1));
      std::copy(str.begin(), str.end(), s_);
      size_ = str.size();
      s_[size_] = 0;
   }
   
   pstring& pstring::operator = (const pstring& rhs)
   {
      s_ = rhs.s_;
      size_ = rhs.size_;

      return *this;
   }

   std::ostream& operator << (std::ostream& os, const pstring& v)
   {
      if (v.size_)
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

   bool pstring::operator < (const std::string& rhs) const
   {
      if (size_ == rhs.size())
         return strncmp(s_, rhs.c_str(), size_) < 0;
      else
         return size_ < rhs.size();
   }

   std::string pstring::to_string() const
   {
      return std::string(s_, s_ + size_);
   }

   bool operator == (const pstring& lhs, const char* rhs)
   {
      if (lhs.begin() != NULL && rhs != NULL)
         return strcmp(lhs.begin(), rhs) == 0;
      else
         return lhs.begin() == rhs;
   }

   bool operator == (const pstring& lhs, const pstring& rhs)
   {
      return lhs == rhs.begin();
   }

   std::string::size_type pstring::find_last_of(char c) const
   {
      if (empty())
         return std::string::npos;
      
      for(const char* i = end() - 1, *first = begin(); i >= first; --i)
         if (*i == c)
            return i - begin();

      return std::string::npos;
   }

   bool pstring::is_suffix_of(const pstring& source) const
   {
      if (empty() || source.empty())
         return false;

      if (size() > source.size())
         return false;

      for(size_type i = 0; i < size(); ++i)
         if (s_[size_ - i - 1] != source.s_[source.size_ - i - 1])
            return false;

      return true;
   }
}
