#if !defined(h_ab5e0642_d07a_4fde_9332_972f2306020d)
#define h_ab5e0642_d07a_4fde_9332_972f2306020d

#include "build_environment.h"

namespace hammer
{
   class build_environment_impl : public build_environment
   {
      public:
         build_environment_impl(const location_t& cur_dir);
         virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const;
         virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const;
         virtual const location_t& current_directory() const;
         virtual void create_directories(const location_t& dir_to_create) const;
         virtual void remove(const location_t& p) const;
         virtual void copy(const location_t& full_source_path, const location_t& full_destination_path) const;
         virtual bool write_tag_file(const std::string& filename, const std::string& content) const;
         virtual std::auto_ptr<std::ostream> create_output_file(const char* filename, std::ios_base::_Openmode mode) const;
         virtual location_t working_directory(const basic_target& t) const;
         virtual const location_t* cache_directory() const;
      
      private:
         location_t current_directory_;
         location_t cache_directory_;

         void dump_shell_command(std::ostream& s, const location_t& full_content_file_name) const;
         bool run_shell_commands(std::string* captured_output, 
                                 const std::vector<std::string>& cmds, 
                                 const location_t& working_dir) const;
   };
}
#endif //h_ab5e0642_d07a_4fde_9332_972f2306020d
