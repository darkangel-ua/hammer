#if !defined(h_d1629c54_5878_485f_a64b_7230475dc879)
#define h_d1629c54_5878_485f_a64b_7230475dc879

#include "argument_writer.h"
#include <vector>

namespace hammer
{
   class target_type;

   class targets_argument_writer : public argument_writer
   {
      public:
         targets_argument_writer(const std::string& name, const target_type& t);
         const target_type& source_type() const { return *source_type_; }
			std::vector<const feature*> valuable_features() const override { return {}; }

      private:
         const target_type* source_type_;
   };
}

#endif //h_d1629c54_5878_485f_a64b_7230475dc879
