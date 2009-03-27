#if !defined(h_33a5604a_4dc7_4db3_a49d_6e49e5c11d3d)
#define h_33a5604a_4dc7_4db3_a49d_6e49e5c11d3d

#include "argument_writer.h"

namespace hammer
{
   class target_type; 
   class shared_lib_dirs_writer : public argument_writer
   {
      public:
         shared_lib_dirs_writer(const std::string& name,
                                const target_type& shared_lib_type) 
            : argument_writer(name),
              shared_lib_type_(shared_lib_type)
         {}
      
         virtual argument_writer* clone() const { return new shared_lib_dirs_writer(*this); }

      protected:   
         virtual void write_impl(std::ostream& output, 
                                 const build_node& node, 
                                 const build_environment& environment) const;
      private:
         const target_type& shared_lib_type_;
   };
}
#endif //h_33a5604a_4dc7_4db3_a49d_6e49e5c11d3d
