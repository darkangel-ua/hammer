#ifndef HAMMER_CORE_PROXIED_BUILD_ENVIRONMENT_H
#define HAMMER_CORE_PROXIED_BUILD_ENVIRONMENT_H

#include <hammer/core/build_environment.h>
#include <hammer/core/basic_build_target.h>

namespace hammer {

class proxied_build_environment : public build_environment
{
   public:
      proxied_build_environment(const build_environment& env) : env_(env) {}

      bool run_shell_commands(const std::vector<std::string>& cmds,
                              const location_t& working_dir) const override
      {
         return env_.run_shell_commands(cmds, working_dir);
      }

      bool run_shell_commands(std::string& captured_output,
                              const std::vector<std::string>& cmds,
                              const location_t& working_dir) const override
      {
         return env_.run_shell_commands(captured_output, cmds, working_dir);
      }

      bool run_shell_commands(std::ostream& captured_output_stream,
                                      const std::vector<std::string>& cmds,
                                      const location_t& working_dir) const override
      {
         return env_.run_shell_commands(captured_output_stream, cmds, working_dir);
      }

      bool run_shell_commands(std::ostream& captured_output_stream,
                                      std::ostream& captured_error_stream,
                                      const std::vector<std::string>& cmds,
                                      const location_t& working_dir) const override
      {
         return env_.run_shell_commands(captured_output_stream, captured_error_stream, cmds, working_dir);
      }

      const location_t& current_directory() const override
      {
         return env_.current_directory();
      }

      void create_directories(const location_t& dir_to_create) const override
      {
         env_.create_directories(dir_to_create);
      }

      void remove(const location_t& p) const override
      {
         env_.remove(p);
      }

      void remove_file_by_pattern(const location_t& dir,
                                  const std::string& pattern) const override
      {
         env_.remove_file_by_pattern(dir, pattern);
      }

      void copy(const location_t& full_source_path,
                const location_t& full_destination_path) const override
      {
         env_.copy(full_source_path, full_destination_path);
      }

      bool write_tag_file(const std::string& filename,
                          const std::string& content) const override
      {
         return env_.write_tag_file(filename, content);
      }

      std::unique_ptr<std::ostream>
      create_output_file(const char* filename,
                         std::ios_base::openmode mode) const override
      {
         return env_.create_output_file(filename, mode);
      }

      location_t working_directory(const basic_build_target& t) const override
      {
         return env_.working_directory(t);
      }

      std::ostream& output_stream() const override
      {
         return env_.output_stream();
      }

      std::ostream& error_stream() const override
      {
         return env_.error_stream();
      }

      const location_t* cache_directory() const
      {
         return env_.cache_directory();
      }

   protected:
      const build_environment& env_;
};

}

#endif
