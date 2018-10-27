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

struct build_request;

int handle_build_cmd(const std::vector<std::string>& args,
                     const unsigned debug_level,
                     volatile bool& interrupt_flag);
void show_build_cmd_help();

std::unique_ptr<hammer::engine>
setup_engine(const unsigned debug_level,
             const bool configure_toolsets = true);

void resolve_meta_targets(build_request& build_request,
                          const hammer::project* p);

std::vector<hammer::basic_target*>
instantiate(hammer::engine& e,
            const std::vector<const hammer::basic_meta_target*>& meta_targets,
            const hammer::feature_set& build_request);

// generate targets + handle warehouse
boost::optional<hammer::build_nodes_t>
generate(hammer::engine& engine,
         const std::vector<hammer::basic_target*> targets);

