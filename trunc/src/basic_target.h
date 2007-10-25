#pragma once

namespace hammer
{
   class type;
   class basic_target
   {
      public:
         basic_target(const type* t) : type_(t) {}
         const hammer::type* type() const { return type_; }
         virtual ~basic_target(){};
      
      private:
         const hammer::type* type_;
   };
}