#include "stdafx.h"
#include <hammer/parscore/identifier.h>
#include <cstring>
#include <antlr3.h>

namespace hammer{namespace parscore{

identifier::identifier(const char* v)
   : v_(v),
     length_(std::strlen(v)),
     no_lok_(true)
{

}

identifier::identifier(const ANTLR3_COMMON_TOKEN_struct* v)
   : lok_(v),
     v_(NULL),
     length_(v->stop - v->start + 1),
     no_lok_(false)
{
}

identifier::identifier()
   : v_(NULL), 
     no_lok_(true)
{
}

bool identifier::operator < (const identifier& rhs) const
{
   const char* lhs_v = no_lok_ ? v_ : lok_.begin();
   const char* rhs_v = rhs.no_lok_ ? rhs.v_ : rhs.lok_.begin();

   if (length_ != rhs.length_)
      return length_ < rhs.length_;

   return std::strncmp(lhs_v, rhs_v, length_) < 0;
}

bool identifier::operator == (const identifier& rhs) const
{
   if (this == &rhs)
      return true;

   const char* lhs_v = no_lok_ ? v_ : lok_.begin();
   const char* rhs_v = rhs.no_lok_ ? rhs.v_ : rhs.lok_.begin();

   if (length_ != rhs.length_)
      return false;

   return std::strncmp(lhs_v, rhs_v, length_) == 0;
}

std::string identifier::to_string() const
{
   if (no_lok_)
      return std::string(v_, v_ + length_);
   else
      return std::string(lok_.begin(), lok_.begin() + length_);
};

const char* identifier::begin() const
{
   return no_lok_ ? v_ : lok_.begin();
}

const char* identifier::end() const
{
   return no_lok_ ? v_ + length_: lok_.begin() + length_;
}

std::ostream& operator << (std::ostream& os, const identifier& v)
{
   os << v.to_string();
   return os;
}

}}