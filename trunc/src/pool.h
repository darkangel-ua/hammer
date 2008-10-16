#if !defined(h_eaef70c3_2d45_421b_afd6_b9927a12079b)
#define h_eaef70c3_2d45_421b_afd6_b9927a12079b

#include <list>

namespace hammer
{
   class pool
   {
      public:
         pool();
         void* malloc(size_t size);
         ~pool();
      
      private:
         std::list<char*> m_;
   };
}

#endif //h_eaef70c3_2d45_421b_afd6_b9927a12079b
