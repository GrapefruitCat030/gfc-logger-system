cmake_minimum_required(VERSION 3.0)
project(gfc-logger-system)

set(CMAKE_VERBOSE_MAKEFILE ON)  # Enable verbose output for debugging
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++2a -Wall -Wextra -Werror=return-type -Werror=uninitialized -Werror=unused-function --sanitize=address -g")

set(LIBRARY_SOURCES gfc-logger-system/logger.cc)

# Uncomment the following line if you want to build a shared library
add_library(gfc-logger-system SHARED ${LIBRARY_SOURCES})

# Uncomment the following lines if you want to build a static library
# add_library(gfc-logger-system-static STATIC ${LIBRARY_SOURCES})
# set_target_properties(gfc-logger-system-static PROPERTIES OUTPUT_NAME "gfc-logger-system")

add_executable(test_logger tests/test_logger.cc)
add_dependencies(test_logger gfc-logger-system)
target_link_libraries(test_logger gfc-logger-system)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

