#if !defined(h_51b99cdf_4b23_4067_b739_108519363f5b)
#define h_51b99cdf_4b23_4067_b739_108519363f5b

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include "location.h"
#include <boost/noncopyable.hpp>

namespace hammer
{
   namespace details
   {
      class buffered_output_environment;
   }

   class basic_target;

   class build_environment : public boost::noncopyable
   {
      public:
         friend class ::hammer::details::buffered_output_environment;

         build_environment();
         virtual ~build_environment();

         virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const = 0;
         virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const = 0;
         virtual bool run_shell_commands(std::ostream& captured_output_stream,
                                         const std::vector<std::string>& cmds,
                                         const location_t& working_dir) const = 0;
         virtual bool run_shell_commands(std::ostream& captured_output_stream,
                                         std::ostream& captured_error_stream,
                                         const std::vector<std::string>& cmds,
                                         const location_t& working_dir) const = 0;
         virtual const location_t& current_directory() const = 0;
         virtual void create_directories(const location_t& dir_to_create) const = 0;
         virtual void remove(const location_t& p) const = 0;
         virtual void remove_file_by_pattern(const location_t& dir, const std::string& pattern) const = 0;
         virtual void copy(const location_t& full_source_path, const location_t& full_destination_path) const = 0;
         virtual bool write_tag_file(const std::string& filename, const std::string& content) const = 0;
         virtual std::unique_ptr<std::ostream> create_output_file(const char* filename, std::ios_base::openmode mode) const = 0;
         // Calculate temp shell file or .vcproj location. Then we calculate include, output input and others relative to this directory.
         virtual location_t working_directory(const basic_target& t) const = 0;
         virtual std::ostream& output_stream() const = 0;
         virtual std::ostream& error_stream() const = 0;
         // Returns directory where build process counterparts should save/load cashes
         // If returns null - caching is not allowed
         virtual const location_t* cache_directory() const = 0;

      private:
         struct impl_t;
         mutable impl_t* impl_;

         std::ostream& begin_use_output_stream() const;
         void end_use_output_stream(std::ostream& s) const;
   };
}

#endif //h_51b99cdf_4b23_4067_b739_108519363f5b
