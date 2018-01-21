#if !defined(h_ead414ef_0ee0_49a7_b1e0_273358b3010f)
#define h_ead414ef_0ee0_49a7_b1e0_273358b3010f

#include <hammer/core/argument_writer.h>

namespace hammer
{
   class batch_tag_writer : public argument_writer
   {
      public:
         batch_tag_writer() : argument_writer("") {}
         argument_writer* clone() const override { return new batch_tag_writer(*this); }
			std::vector<const feature*> valuable_features() const override { return {}; }

      protected:   
         void write_impl(std::ostream& output,
                         const build_node& node,
                         const build_environment& environment) const override
         {
            output << "batched." << &node;
         };
   };
}

#endif //h_ead414ef_0ee0_49a7_b1e0_273358b3010f
