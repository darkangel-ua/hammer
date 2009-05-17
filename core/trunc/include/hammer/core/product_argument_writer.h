#if !defined(h_a7ca86c5_dbb5_4306_82c3_a5718516cf52)
#define h_a7ca86c5_dbb5_4306_82c3_a5718516cf52

#include "targets_argument_writer.h"

namespace hammer
{
   class product_argument_writer : public targets_argument_writer
   {
      public:
         product_argument_writer(const std::string& name, const target_type& t);
         virtual argument_writer* clone() const;

      protected:
         virtual void write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const;
   };
}

#endif //h_a7ca86c5_dbb5_4306_82c3_a5718516cf52