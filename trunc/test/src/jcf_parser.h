#if !defined(h_38a1b5fc_c4b5_443f_9fe8_8099d3855f65)
#define h_38a1b5fc_c4b5_443f_9fe8_8099d3855f65

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

namespace hammer
{
   class basic_target;
   class engine;
}

class jcf_parser : public boost::noncopyable
{
   public:
      jcf_parser();
      bool parse(const char* file_name);
      bool parse(const boost::filesystem::path& file_name);
      bool walk(const std::vector<hammer::basic_target*>& targets, hammer::engine* e);
      ~jcf_parser() { reset(); }

   private:
      struct impl_t;
      impl_t* impl_;

      void reset();
};

#endif //h_38a1b5fc_c4b5_443f_9fe8_8099d3855f65
