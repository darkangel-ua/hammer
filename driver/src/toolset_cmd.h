#pragma once

#include <vector>
#include <string>

int handle_toolset_cmd(const std::vector<std::string>& args,
                       const unsigned debug_level);

void show_toolset_cmd_help();
