#if !defined(h_7b291f21_f482_4c1a_be16_3d16119ed90a)
#define h_7b291f21_f482_4c1a_be16_3d16119ed90a

#include <hammer/core/build_environment.h>

namespace hammer{namespace details{

class buffered_output_environment : public build_environment
{
   public:
      buffered_output_environment(const hammer::build_environment& env) 
         : env_(env), 
           s_(env.begin_use_output_stream())
      {}

      ~buffered_output_environment() { env_.end_use_output_stream(s_); }

      virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const
      {
         return env_.run_shell_commands(cmds, working_dir);
      }

      virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const
      {
         return env_.run_shell_commands(captured_output, cmds, working_dir);
      }

      virtual bool run_shell_commands(std::ostream& captured_output_stream, const std::vector<std::string>& cmds, const location_t& working_dir) const
      {
         return env_.run_shell_commands(captured_output_stream, cmds, working_dir);
      }

      virtual const location_t& current_directory() const { return env_.current_directory(); }
      virtual void create_directories(const location_t& dir_to_create) const { env_.create_directories(dir_to_create); }
      virtual void remove(const location_t& p) const { env_.remove(p); }
      
      virtual void remove_file_by_pattern(const location_t& dir, const std::string& pattern) const
      {
         env_.remove_file_by_pattern(dir, pattern);
      }

      virtual void copy(const location_t& full_source_path, const location_t& full_destination_path) const
      {
         env_.copy(full_source_path, full_destination_path);
      }

      virtual bool write_tag_file(const std::string& filename, const std::string& content) const
      {
         return env_.write_tag_file(filename, content);
      }

      virtual std::auto_ptr<std::ostream> create_output_file(const char* filename, std::ios_base::openmode mode) const
      {
         return env_.create_output_file(filename, mode);
      }

      virtual location_t working_directory(const basic_target& t) const { return env_.working_directory(t); }
      virtual const location_t* cache_directory() const { return env_.cache_directory(); }
      virtual std::ostream& output_stream() const
      {
         return s_;
      }

   private:
      const hammer::build_environment& env_;
      std::ostream& s_;

      buffered_output_environment(const buffered_output_environment& rhs);
};

}}

#endif //h_7b291f21_f482_4c1a_be16_3d16119ed90a
