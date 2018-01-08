#if !defined(h_70de0d29_8bee_4d04_915c_62c54b1e65e8)
#define h_70de0d29_8bee_4d04_915c_62c54b1e65e8

#include <hammer/core/scaner.h>
#include <hammer/core/hashed_location.h>
#include <vector>
#include <utility>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/weak_ptr.hpp>

namespace hammer
{
   struct c_scanner_context;
   class c_scanner : public scanner
   {
         friend struct c_scanner_context;      

      public:
         c_scanner() : scanner("C scanner") {}
         boost::posix_time::ptime process(const basic_build_target& t,
														scanner_context& context) const override;
         boost::shared_ptr<scanner_context> create_context(const build_environment& env) const override;
      
      private:
         mutable boost::weak_ptr<scanner_context> context_;
   };
}

#endif //h_70de0d29_8bee_4d04_915c_62c54b1e65e8
