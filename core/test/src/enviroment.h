#pragma once
#include <functional>
#include <boost/filesystem/path.hpp>
#include <hammer/core/engine.h>

extern boost::filesystem::path test_data_path;

struct setuped_engine {
   setuped_engine(bool install_toolsets = true);

   hammer::engine engine_;
};

struct complete_build_tests_environment {
   complete_build_tests_environment();
   void install_toolsets();

   void run_test(const boost::filesystem::path& working_dir);

   hammer::engine engine_;
};

// test_data_path pointing to top directoriy where for each test we create individual directory
// with concreate structure and hamfile. Hamfile should contain 'test' meta target that will be builded
void add_tests_from_filesystem(const boost::filesystem::path& test_data_path,
                               const std::string& test_suite_name,
                               std::function<void (const boost::filesystem::path& working_dir)> test_function);
