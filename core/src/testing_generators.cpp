#include "stdafx.h"
#include <hammer/core/testing_generators.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/testing_run_action.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/shared_lib_dirs_writer.h>
#include <hammer/core/free_feature_arg_writer.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/compile_fail_generator.h>

using std::unique_ptr;
using namespace boost;

namespace hammer{

void add_testing_generators(engine& e, generator_registry& gr)
{
   generator::consumable_types_t source;
   generator::producable_types_t target;
   source.push_back(generator::consumable_type(e.get_type_registry().get(types::EXE), 1, 0));
   // FIXME: I add those here because in case when we specify <library>/foo for testing project to be added to all tests
   //        but this will also add static/shared lib to testing run generator, and because there is generator from static/shared
   //        lib to EXE it will try to build EXE from lib and than feed it to testing_run target
   //        I need to find a better way to deal with unimportant sources or with project features propagation
   source.push_back(generator::consumable_type(e.get_type_registry().get(types::STATIC_LIB)));
   source.push_back(generator::consumable_type(e.get_type_registry().get(types::SHARED_LIB)));

   target.push_back(generator::produced_type(e.get_type_registry().get(types::TESTING_OUTPUT)));
   target.push_back(generator::produced_type(e.get_type_registry().get(types::TESTING_RUN_PASSED)));

   shared_ptr<product_argument_writer> run_product(new product_argument_writer("run_product", e.get_type_registry().get(types::TESTING_RUN_PASSED)));
   shared_ptr<product_argument_writer> run_output_product(new product_argument_writer("run_output_product", e.get_type_registry().get(types::TESTING_OUTPUT)));
   shared_ptr<source_argument_writer> test_executable(new source_argument_writer("test_executable", e.get_type_registry().get(types::EXE)));
   shared_ptr<shared_lib_dirs_writer> additional_dirs(new shared_lib_dirs_writer("additional_dirs", e.get_type_registry().get(types::SHARED_LIB)));
   shared_ptr<free_feature_arg_writer> input_files(new free_feature_arg_writer("input_files", e.feature_registry(), "testing.input-file"));
   shared_ptr<free_feature_arg_writer> args(new free_feature_arg_writer("args", e.feature_registry(), "testing.argument"));
#if defined(_WIN32)
   cmdline_builder cmdline("@SET PATH=%PATH%;$(additional_dirs)\n"
                           "@$(test_executable) $(args) $(input_files)\n");
#else
   cmdline_builder cmdline("export LD_LIBRARY_PATH=$(additional_dirs):$LD_LIBRARY_PATH\n"
                           "$(test_executable) $(args) $(input_files)");
#endif
   cmdline += run_product;
   cmdline += run_output_product;
   cmdline += test_executable;
   cmdline += additional_dirs;
   cmdline += input_files;
   cmdline += args;

   auto action = std::make_shared<testing_run_action>("testing.run", run_product, run_output_product);
   *action += cmdline;
   unique_ptr<generator> g(new generator(e, "testing.run", source, target, true, action));
   g->include_composite_generators(true);
   gr.insert(std::move(g));
}

}
