#pragma once

#include <vector>
#include <string>

int handle_clean_cmd(const std::vector<std::string>& args,
                     const unsigned debug_level);

void show_clean_cmd_help();
