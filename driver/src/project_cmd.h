#pragma once

#include <vector>
#include <string>

int handle_project_cmd(const std::vector<std::string>& args,
                       const unsigned debug_level);

void show_project_cmd_help();
