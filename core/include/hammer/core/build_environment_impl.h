#pragma once
#include <memory>
#include <thread>
#include <boost/asio/executor_work_guard.hpp>
#include <hammer/core/build_environment.h>

namespace hammer {

class build_environment_impl : public build_environment {
   public:
      build_environment_impl(const location_t& cur_dir,
                             const bool print_shell_commands = false);
      ~build_environment_impl() override;

		const location_t& current_directory() const override;
      void create_directories(const location_t& dir_to_create) const override;
      void remove(const location_t& p) const override;
      void remove_file_by_pattern(const location_t& dir,
		                            const std::string& pattern) const override;
      void copy(const location_t& full_source_path,
		          const location_t& full_destination_path) const override;
      bool write_tag_file(const std::string& filename,
		                    const std::string& content) const override;
      std::unique_ptr<std::ostream>
		create_output_file(const char* filename,
		                   std::ios_base::openmode mode) const override;
      location_t working_directory(const basic_build_target& t) const override;
      std::ostream& output_stream() const override;
      std::ostream& error_stream() const override;
      const location_t* cache_directory() const override;

   private:
      location_t current_directory_;
      const bool print_shell_commands_;
      location_t cache_directory_;
      std::auto_ptr<std::ostream*> output_stream_;
      std::auto_ptr<std::ostream*> error_stream_;
      mutable boost::asio::io_context shell_executor_;
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type> shell_executor_guard_;
      std::thread shell_executor_thread_;

      // FIXME: need to find better name
      void dump_shell_command(std::ostream& s, const location_t& full_content_file_name) const;
      bool run_shell_commands(std::ostream* captured_output_stream,
                              std::ostream* captured_error_stream,
                              const std::vector<std::string>& cmds,
                              const location_t& working_dir) const;
      virtual
      boost::asio::io_context& shell_executor() const { return shell_executor_; }
};

}
