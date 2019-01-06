#pragma once

#include <vector>
#include <string>

int handle_package_cmd(std::vector<std::string> args,
                       const unsigned debug_level);

void show_package_cmd_help(const std::vector<std::string>& args);
