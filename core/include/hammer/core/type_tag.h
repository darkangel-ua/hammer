#if !defined(h_6ce02cfc_cbaf_4b9b_986f_463f51485c44)
#define h_6ce02cfc_cbaf_4b9b_986f_463f51485c44

#include <string>

namespace hammer
{
   class type_tag 
   {
      public:
         explicit type_tag(const std::string& name) : name_(name) {}
         const std::string& name() const { return name_; }
         bool operator == (const type_tag& rhs) const { return name() == name(); }
         bool operator < (const type_tag& rhs) const { return name() < rhs.name(); }

      private:
         std::string name_;
   };
}

#endif //h_6ce02cfc_cbaf_4b9b_986f_463f51485c44
