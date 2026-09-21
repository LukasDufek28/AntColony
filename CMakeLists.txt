cmake_minimum_required(VERSION 3.20)
project(AntEvolution LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  add_executable(AntEvolution WIN32 src/main_win32.cpp)
  target_compile_definitions(AntEvolution PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
  target_compile_options(AntEvolution PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/O2 /arch:AVX2>)
  target_link_libraries(AntEvolution PRIVATE user32 gdi32)
else()
  add_executable(sim_test tests/sim_test.cpp)
endif()
