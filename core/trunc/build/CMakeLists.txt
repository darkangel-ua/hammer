project(hammer.core)

include_directories(../src ../include)

set(c_as_cpp_src ../src/build/hammerLexer.c ../src/build/hammerParser.c)
set_source_files_properties(${c_as_cpp_src} PROPERTIES LANGUAGE CXX)
file(GLOB main_sources ../src/*.cpp)
file(GLOB toolsets_sources ../src/toolsets/*.cpp)
file(GLOB project_generators_sources ../src/project_generators/*.cpp)
add_library(hammer.core STATIC ${c_as_cpp_src} ../src/build/hammer_walker.c ${main_sources} ${toolsets_sources} ${project_generators_sources})
#target_link_libraries(hammer.core antlr3c boost.smart_ptr boost.exception boost.function boost.function_types boost.ptr_container boost.asio boost.unordered boost.multi_index)
target_link_libraries(hammer.core antlr3c boost.all pthread boost.crypto boost.process boost.guid)
target_include_directories(hammer.core PUBLIC ../include)

