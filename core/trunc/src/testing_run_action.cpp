#include "stdafx.h"
#include <hammer/core/testing_run_action.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/build_node.h>
#include <hammer/core/main_target.h>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;
using namespace boost::filesystem;

namespace hammer{

bool testing_run_action::run_shell_commands(const std::vector<std::string>& commands,
                                            const build_node& node, 
                                            const build_environment& environment) const
{
   if (node.products_.empty())
      throw std::runtime_error("[testing_run_action] Can't run command for node without products.");
   
   ostringstream output_name;
   output_writer_->write(output_name, node, environment);
   string run_tag_name(target_tag(node, environment));
   
   string output;
   bool result = environment.run_shell_commands(output, commands, node.products_.front()->get_main_target()->location());
   environment.write_tag_file(output_name.str() + ".output", output);
   std::cerr << output << std::flush;

   if (result)
      environment.write_tag_file(run_tag_name, "passed");

   return result;
}

}
