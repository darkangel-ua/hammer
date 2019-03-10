#pragma once
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <hammer/core/build_node.h>

namespace hammer {
   class engine;
   class project;
   class basic_target;
   class basic_meta_target;
   class feature_set;
}

struct build_request_t;

int handle_build_cmd(const std::vector<std::string>& args,
                     const unsigned debug_level,
                     volatile bool& interrupt_flag);
void show_build_cmd_help();

std::unique_ptr<hammer::engine>
setup_engine(const unsigned debug_level,
             const bool configure_toolsets = true);

std::vector<hammer::basic_target*>
instantiate(hammer::engine& e,
            const std::vector<const hammer::basic_meta_target*>& meta_targets,
            const hammer::feature_set& build_request_t);

// generate targets + handle warehouse
boost::optional<hammer::build_nodes_t>
generate(hammer::engine& engine,
         const std::vector<hammer::basic_target*> targets);

void build(hammer::engine& e,
           hammer::build_nodes_t& nodes,
           const unsigned debug_level,
           volatile bool& interrupt_flag );
