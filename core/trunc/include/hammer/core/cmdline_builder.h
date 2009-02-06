#if !defined(h_754d46d2_cd26_4948_861c_c0e0096ac358)
#define h_754d46d2_cd26_4948_861c_c0e0096ac358

#include <string>
#include <memory>
#include "argument_writer.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/smart_cast.hpp>

namespace hammer
{
   class build_environment;

   class cmdline_builder
   {
      public:
         cmdline_builder(const std::string& cmd);
         template<typename T>
         cmdline_builder& operator +=(boost::shared_ptr<T>& v) { add(boost::shared_static_cast<argument_writer>(v)); return *this; }
         void write(std::ostream& output, const build_node& node, const build_environment& environment) const;

      private:
         typedef std::map<const std::string, boost::shared_ptr<argument_writer> > writers_t;

         std::string cmd_;
         writers_t writers_;

         void add(const boost::shared_ptr<argument_writer>& v);
   };
}

#endif //h_754d46d2_cd26_4948_861c_c0e0096ac358
