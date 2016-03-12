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
//         typedef std::vector<std::pair<hashed_location, bool /* true == #include <> */> > included_files_t;

         c_scanner() : scanner("C scanner") {}
         virtual boost::posix_time::ptime process(const basic_target& t, 
                                                  scanner_context& context) const;
         virtual boost::shared_ptr<scanner_context> create_context(const build_environment& env) const;
      
      protected:
         // WARNING! Implementation is required to cache extracted data
//          virtual const included_files_t& extract_includes(const location_t& file, 
//                                                           const boost::posix_time::ptime& file_timestamp, 
//                                                           const c_scanner_context& context) const;
      private:
         mutable boost::weak_ptr<scanner_context> context_;
   };
}

#endif //h_70de0d29_8bee_4d04_915c_62c54b1e65e8
