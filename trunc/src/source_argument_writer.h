#if !defined(h_ab2aca5e_e63f_423c_b6da_5ac29570cf00)
#define h_ab2aca5e_e63f_423c_b6da_5ac29570cf00

#include "targets_argument_writer.h"

namespace hammer
{
   class source_argument_writer : public targets_argument_writer
   {
      public:
         source_argument_writer(const std::string& name, const type& t);
         virtual argument_writer* clone() const;

      protected:
         virtual void write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const;
   };
}

#endif //h_ab2aca5e_e63f_423c_b6da_5ac29570cf00
