#if !defined(h_d1629c54_5878_485f_a64b_7230475dc879)
#define h_d1629c54_5878_485f_a64b_7230475dc879

#include "argument_writer.h"
#include <vector>

namespace hammer
{
   class type;

   class targets_argument_writer : public argument_writer
   {
      public:
         targets_argument_writer(const std::string& name, const type& t);
         const type& source_type() const { return *source_type_; }

      private:
         const type* source_type_;
   };
}

#endif //h_d1629c54_5878_485f_a64b_7230475dc879
