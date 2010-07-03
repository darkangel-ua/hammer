#if !defined(h_25c0e67c_ed8e_4fb9_9da4_e8bfb9aff439)
#define h_25c0e67c_ed8e_4fb9_9da4_e8bfb9aff439

#include <boost/filesystem/path.hpp>
#include <hammer/sema/actions.h>

namespace hammer{namespace parser{

class parser
{
   public:
      static void parse(const boost::filesystem::path& hamfile,
                        const sema::actions& actions);

   private:
      parser(const boost::filesystem::path& hamfile,
             const sema::actions& actions);
      void parse_impl();

      boost::filesystem::path hamfile_;
      const sema::actions& actions_;
};

}}
#endif //h_25c0e67c_ed8e_4fb9_9da4_e8bfb9aff439
