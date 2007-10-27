#if !defined(h_2c18b3a3_2980_49f4_bd29_c1f55088957a)
#define h_2c18b3a3_2980_49f4_bd29_c1f55088957a

#include <boost/noncopyable.hpp>
#include <vector>

namespace hammer
{
   class engine;
   class basic_target;
   class feature_set;

   namespace project_generators 
   {
      class msvc : public boost::noncopyable
      {
         public:
            struct impl_t;

            msvc(engine& e);
            void add_variant(const std::vector<basic_target*>& targets, 
                             const feature_set& variant);
            void generate();
            ~msvc();

         private:
            impl_t* impl_;
      };
   }
}

#endif //h_2c18b3a3_2980_49f4_bd29_c1f55088957a
