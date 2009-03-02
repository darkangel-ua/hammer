#if !defined(h_4b7030c3_5a8e_4b2b_8c28_74e6c2a4788c)
#define h_4b7030c3_5a8e_4b2b_8c28_74e6c2a4788c

#include "targets_argument_writer.h"

namespace hammer
{
   class output_dir_argument_writer : public argument_writer
   {
      public:
         output_dir_argument_writer(const std::string& name) : argument_writer(name) {};
         virtual argument_writer* clone() const { return new output_dir_argument_writer(*this); }

      protected:
         virtual void write_impl(std::ostream& output, 
                                 const build_node& node, 
                                 const build_environment& environment) const;
   };
}

#endif //h_4b7030c3_5a8e_4b2b_8c28_74e6c2a4788c
