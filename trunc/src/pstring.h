#if !defined(h_8a1631c9_fa60_41d1_9cb8_283ab6090d1b)
#define h_8a1631c9_fa60_41d1_9cb8_283ab6090d1b

#include <iosfwd>
#include <string>

namespace hammer
{
   class pool;
   class pstring
   {
      public:
         typedef unsigned int size_type;
         pstring() : s_(0), size_(0) {}
         pstring(pool& p, const char* first, const char* last);
         pstring(pool& p, const char* str);
         pstring(pool& p, const std::string& str);
         size_type size() const { return size_; }
         bool empty() const { return s_ == NULL || size_ == 0; }
         std::string to_string() const;
         const char* begin() const { return s_; }
         const char* end() const { return s_ + size_; }
         pstring& operator = (const pstring& rhs);
         ~pstring(){};
         friend std::ostream& operator << (std::ostream& os, const pstring& rhs);
         bool operator < (const pstring& rhs) const;
         bool operator < (const std::string& rhs) const;
         std::string::size_type find_last_of(char c) const;

      private:
         char* s_;
         size_type size_;
   };

   bool operator == (const pstring& lhs, const char* rhs);
   bool operator == (const pstring& lhs, const pstring& rhs);
   inline bool operator == (const std::string& lhs, const pstring& rhs) { return rhs == lhs.c_str(); }
   inline bool operator == (const pstring& lhs, const std::string& rhs) { return lhs == rhs.c_str(); }
   inline bool operator != (const pstring& lhs, const std::string& rhs) { return !(lhs == rhs); }
   inline bool operator != (const std::string& lhs, const pstring& rhs) { return !(rhs == lhs); }
   inline bool operator != (const pstring& lhs, const char* rhs) { return !(lhs == rhs); }
   inline bool operator != (const pstring& lhs, const pstring& rhs) { return !(lhs == rhs.begin()); }
}

#endif //h_8a1631c9_fa60_41d1_9cb8_283ab6090d1b
