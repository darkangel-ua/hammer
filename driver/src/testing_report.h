#pragma once
#include <boost/filesystem/path.hpp>
#include <hammer/core/build_node.h>

void generate_testing_report(const boost::filesystem::path& report_filename,
                             const hammer::build_nodes_t& nodes);
