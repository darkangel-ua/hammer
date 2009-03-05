#if !defined(h_51b99cdf_4b23_4067_b739_108519363f5b)
#define h_51b99cdf_4b23_4067_b739_108519363f5b

#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include "location.h"

namespace hammer
{
   class build_environment
   {
      public:
         virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const = 0;
         virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const = 0;
         virtual const location_t& current_directory() const = 0;
         virtual void create_directories(const location_t& dir_to_create) const = 0;
         virtual void remove(const location_t& p) const = 0;
         virtual void copy(const location_t& full_source_path, const location_t& full_destination_path) const = 0;
         virtual bool write_tag_file(const std::string& filename, const std::string& content) const = 0;
         virtual std::auto_ptr<std::ostream> create_output_file(const char* filename, std::ios_base::_Openmode mode) const = 0;
         virtual ~build_environment() {}
   };
}

#endif //h_51b99cdf_4b23_4067_b739_108519363f5b
