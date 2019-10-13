#include <hammer/core/build_environment.h>

namespace hammer {

bool build_environment::run_shell_commands(const std::vector<std::string>& cmds,
                                           const location_t& working_dir) const {
   return run_shell_commands(nullptr, nullptr, cmds, working_dir);
}

bool build_environment::run_shell_commands(std::string& captured_output,
                                           const std::vector<std::string>& cmds,
                                           const location_t& working_dir) const {
   std::stringstream s;
   bool result = run_shell_commands(&s, nullptr, cmds, working_dir);
   captured_output = s.str();
   return result;
}

bool build_environment::run_shell_commands(std::ostream& captured_output_stream,
                                           const std::vector<std::string>& cmds,
                                           const location_t& working_dir) const {
   return run_shell_commands(&captured_output_stream, nullptr, cmds, working_dir);
}

bool build_environment::run_shell_commands(std::ostream& captured_output_stream,
                                           std::ostream& captured_error_stream,
                                           const std::vector<std::string>& cmds,
                                           const location_t& working_dir) const {
   return run_shell_commands(&captured_output_stream, &captured_error_stream, cmds, working_dir);
}

}
