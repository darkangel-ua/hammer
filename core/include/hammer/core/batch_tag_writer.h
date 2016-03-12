#if !defined(h_ead414ef_0ee0_49a7_b1e0_273358b3010f)
#define h_ead414ef_0ee0_49a7_b1e0_273358b3010f

#include <hammer/core/argument_writer.h>

namespace hammer
{
   class batch_tag_writer : public argument_writer
   {
      public:
         batch_tag_writer() : argument_writer("") {}
         virtual argument_writer* clone() const { return new batch_tag_writer(*this); }

      protected:   
         virtual void write_impl(std::ostream& output, 
                                 const build_node& node, 
                                 const build_environment& environment) const
         {
            output << "batched." << &node;
         };
   };
}

#endif //h_ead414ef_0ee0_49a7_b1e0_273358b3010f
