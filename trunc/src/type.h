#if !defined(h_911b15ef_ec54_4217_bee8_b61c7584f9f5)
#define h_911b15ef_ec54_4217_bee8_b61c7584f9f5

#include <string>
#include <vector>

namespace hammer
{
   class type
   {
      public:
         typedef std::vector<std::string> suffixes_t;
         type(const std::string& name, const std::string& suffix, type* base = 0);
         type(const std::string& name, const suffixes_t& suffixes, type* base = 0);
         const std::string& name() const { return name_; }
         const suffixes_t& suffixes() const { return suffixes_; }
         const std::string& suffix_for(const std::string& s) const;
         bool operator == (const type& rhs) const { return name() == rhs.name(); }
         bool operator != (const type& rhs) const { return !(*this == rhs); }

      private:
         std::string name_;
         suffixes_t suffixes_;
         type* base_;
   };
}

#endif //h_911b15ef_ec54_4217_bee8_b61c7584f9f5
