#if !defined(h_754d46d2_cd26_4948_861c_c0e0096ac358)
#define h_754d46d2_cd26_4948_861c_c0e0096ac358

#include <string>
#include <memory>
#include "argument_writer.h"
#include <boost/ptr_container/ptr_map.hpp>

namespace hammer
{
   class build_environment;

   class cmdline_builder
   {
      public:
         cmdline_builder(const std::string& cmd);
         cmdline_builder(const cmdline_builder& lhs);
         template<typename T>
         cmdline_builder& operator +=(std::auto_ptr<T>& v) { add(v.get()); v.release(); return *this; }
         cmdline_builder& operator =(const cmdline_builder& rhs);
         void write(std::ostream& output, const build_node& node, const build_environment& environment) const;

      private:
         typedef boost::ptr_map<const std::string, argument_writer> writers_t;

         std::string cmd_;
         writers_t writers_;

         void add(argument_writer* v);
   };
}

#endif //h_754d46d2_cd26_4948_861c_c0e0096ac358
