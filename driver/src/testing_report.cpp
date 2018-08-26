#include "testing_report.h"
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;
using namespace hammer;

static
build_nodes_t
gather_testing_nodes(const hammer::build_nodes_t& nodes) {
   build_nodes_t result;

   return result;
}

void generate_testing_report(const fs::path& report_filename,
                             const hammer::build_nodes_t& nodes) {
   fs::ofstream f(report_filename, std::ios_base::trunc);
   if (!f)
      throw std::runtime_error("Failed to open '" + report_filename.string() + "' for writing");

   const auto testing_nodes = gather_testing_nodes(nodes);
}
