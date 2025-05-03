cmake_minimum_required(VERSION 3.26)
project(Tasks)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

add_compile_definitions(PQXX_NO_SOURCE_LOCATION)

include_directories(${PQXX_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS})

add_executable(Tasks main.cpp Functions.cpp)
target_link_libraries(Tasks ${PQXX_LIBRARIES} pq)
