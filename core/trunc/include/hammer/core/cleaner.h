#if !defined(h_64d0356a_8cf7_4eb2_911f_6dd161a13741)
#define h_64d0356a_8cf7_4eb2_911f_6dd161a13741

#include <hammer/core/build_node.h>

namespace hammer
{
   class engine;
   class build_environment;

   class cleaner
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
         
         struct result
         {
            result() : cleaned_target_count_(0),
                       failed_to_clean_target_count_(0)
            {}

            unsigned cleaned_target_count_;
            unsigned failed_to_clean_target_count_;
         };

         cleaner(engine& e, const build_environment& environment);
         result clean_all(nodes_t& nodes);
      
      private:
         struct impl_t;
         impl_t* impl_;
   };
}

#endif //h_64d0356a_8cf7_4eb2_911f_6dd161a13741
