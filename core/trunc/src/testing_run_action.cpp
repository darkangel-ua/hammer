#include "stdafx.h"
#include "testing_run_action.h"
#include "build_environment.h"
#include <sstream>
#include <fstream>

using namespace std;
using namespace boost::filesystem;

namespace hammer{

bool testing_run_action::run_shell_commands(const std::vector<std::string>& commands,
                                            const build_node& node, 
                                            const build_environment& environment) const
{
   ostringstream output_name;
   output_writer_->write(output_name, node, environment);
   string run_tag_name(target_tag(node, environment));
   
   string output;
   bool result = environment.run_shell_commands(output, commands);
   if (result)
   {
      environment.write_tag_file(output_name.str(), output);
      environment.write_tag_file(run_tag_name, "passed");
   }

   return result;
}

}
