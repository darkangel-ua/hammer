project(hammer.driver)

file(GLOB sources ./../src/*.cpp)
add_executable(hammer ${sources})
target_link_libraries(hammer boost.all hammer.core)

