#if !defined(h_e7c984c0_a4f6_42e2_89b9_ec70352d581c)
#define h_e7c984c0_a4f6_42e2_89b9_ec70352d581c

#include "basic_target.h"
#include "pstring.h"
#include <vector>

namespace hammer
{
   class main_target : public basic_target
   {
      public:
         typedef std::vector<basic_target*> sources_t;

         main_target(const pstring& name, const hammer::type* t);
         void* operator new(size_t size) { return new char[size]; }
         void sources(const std::vector<basic_target*>& srcs);
         const sources_t& sources() const { return sources_; }

      private:
         sources_t sources_;
   };
}

#endif //h_e7c984c0_a4f6_42e2_89b9_ec70352d581c
