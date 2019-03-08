#pragma once
#include <vector>
#include <string>

int handle_test_cmd(const std::vector<std::string>& args,
                    const unsigned debug_level,
                    volatile bool& interrupt_flag);

void show_test_cmd_help();
