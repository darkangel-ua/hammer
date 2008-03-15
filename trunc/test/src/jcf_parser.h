#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace hammer
{
   class project;
}

class jcf_parser : public boost::noncopyable
{
   public:
      jcf_parser();
      bool parse(const char* file_name);
      bool parse(const boost::filesystem::path& file_name);
      bool walk(const hammer::project* p);
      ~jcf_parser() { reset(); }

   private:
      struct impl_t;
      impl_t* impl_;

      void reset();
};
