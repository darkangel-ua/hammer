#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

namespace hammer {

class basic_target;
class engine;

}

class jcf_parser : public boost::noncopyable {
   public:
      jcf_parser();
      bool parse(const char* file_name);
      bool parse(const boost::filesystem::path& file_name);
      void walk(const std::vector<hammer::basic_target*>& targets, hammer::engine* e);
      ~jcf_parser() { reset(); }

   private:
      struct impl_t;
      impl_t* impl_;

      void reset();
};
