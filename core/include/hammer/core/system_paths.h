#pragma once
#include <boost/filesystem/path.hpp>

namespace hammer {

struct system_paths {
   boost::filesystem::path config_file_;
   boost::filesystem::path data_folder_;
};

// data folder will be created if not exists
system_paths
get_system_paths();

}
