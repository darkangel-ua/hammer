#if !defined(h_bb150578_d78f_4f20_a6e8_3187efa86c30)
#define h_bb150578_d78f_4f20_a6e8_3187efa86c30

#include <string>
#include <memory>
#include <hammer/core/scaner_context.h>
#include <boost/date_time/posix_time/ptime.hpp>

namespace hammer
{
   class basic_target;
   class scanner
   {
      public:
         scanner(const std::string& name) : name_(name) 
         {}

         virtual boost::posix_time::ptime process(const basic_target& t, 
                                                  scanner_context& context) const = 0;
         virtual std::auto_ptr<scanner_context> create_context() const = 0;
         const std::string& name() const { return name_; }
         virtual ~scanner() {}

      private:
         std::string name_;
   };
}

#endif //h_bb150578_d78f_4f20_a6e8_3187efa86c30
