#if !defined(h_70de0d29_8bee_4d04_915c_62c54b1e65e8)
#define h_70de0d29_8bee_4d04_915c_62c54b1e65e8

#include <hammer/core/scaner.h>
#include <hammer/core/hashed_location.h>
#include <vector>
#include <utility>

namespace hammer
{
   struct c_scanner_context;
   class c_scanner : public scanner
   {
         friend struct c_scanner_context;      

      public:
         typedef std::vector<std::pair<hashed_location, bool /* true == #include <> */> > included_files_t;

         c_scanner() : scanner("C scanner") {}
         virtual boost::posix_time::ptime process(const basic_target& t, 
                                                   scanner_context& context) const;
         virtual std::auto_ptr<scanner_context> create_context() const;
      
      protected:
         virtual included_files_t extract_includes(const location_t& file, 
                                                   const c_scanner_context& context) const;
   };
}

#endif //h_70de0d29_8bee_4d04_915c_62c54b1e65e8
