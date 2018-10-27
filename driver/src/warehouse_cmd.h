#pragma once

#include <vector>
#include <string>

int handle_warehouse_cmd(const std::vector<std::string>& args,
                         const unsigned debug_level);

void show_warehouse_cmd_help();
