#if !defined(h_2c18b3a3_2980_49f4_bd29_c1f55088957a)
#define h_2c18b3a3_2980_49f4_bd29_c1f55088957a

#include <boost/noncopyable.hpp>

namespace hammer
{
   class engine;
   class basic_target;

   namespace project_generators 
   {
      class msvc : public boost::noncopyable
      {
         public:
            msvc(const engine& e);
            void generate(const basic_target& t);
            ~msvc();

         private:
            struct impl_t;
            impl_t* impl_;
      };
   }
}

#endif //h_2c18b3a3_2980_49f4_bd29_c1f55088957a
