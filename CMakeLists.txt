cmake_minimum_required (VERSION 3.15)
project (singular-parallel LANGUAGES C CXX)
enable_testing()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

find_package (GPISpace REQUIRED)

find_package (util-cmake REQUIRED)
include (util-cmake/add_macros)
include (util-cmake/beautify_find_boost)

find_boost (1.61 REQUIRED QUIET FROM_GPISPACE_INSTALLATION COMPONENTS
  date_time
  filesystem
  program_options
  regex
  serialization
  system
  test_exec_monitor
  thread
  unit_test_framework
  timer
  iostreams
  chrono
)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/modules")
find_package (Singular REQUIRED)

set (BUNDLE_ROOT "libexec/bundle")
bundle_GPISpace (DESTINATION "${BUNDLE_ROOT}/gpispace"
  COMPONENTS "runtime"
             "monitoring"
  INSTALL_DO_NOT_BUNDLE
)

install (DIRECTORY "${SINGULAR_HOME}/"
  DESTINATION libexec/bundle/singular
  USE_SOURCE_PERMISSIONS
)


set(SP_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
set(PFD_SING_LIB_PATH "${CMAKE_INSTALL_PREFIX}/LIB")
configure_file("src/installation.hpp.in" "src/installation.hpp")

include_directories ("src")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories ("${CMAKE_CURRENT_BINARY_DIR}/src")
include_directories ("${CMAKE_BINARY_DIR}")

extended_add_library (NAME installation
  TYPE SHARED
  NAMESPACE SINGULAR-parallel
  SOURCES "src/installation.cpp"
  LIBRARIES Util::Generic
            Util::Generic-Headers
            GPISpace::execution
  POSITION_INDEPENDENT
  INSTALL
)

set_target_properties (SINGULAR-parallel-installation
  PROPERTIES
  INSTALL_RPATH_USE_LINK_PATH TRUE
)
bundle_GPISpace_add_rpath (TARGET SINGULAR-parallel-installation INSTALL_DIRECTORY "lib")

extended_add_library (NAME module
  TYPE SHARED
  NAMESPACE SINGULAR-parallel
  SOURCES "src/bin/SINGULAR-parallel-module.cpp" "interface/singular_functions.cpp"
  COMPILE_DEFINITIONS SING_NDEBUG OM_NDEBUG
  LIBRARIES SINGULAR-parallel::installation
            partial_static_parallel_allpfd_wrapper
            Util::Generic
            Util::Generic-Headers
            GPISpace::execution
            GPISpace::workflow_development
            Singular
  POSITION_INDEPENDENT
  INSTALL
  INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/LIB"
)
set_target_properties (SINGULAR-parallel-module
  PROPERTIES
  INSTALL_RPATH "$ORIGIN/../lib"
  INSTALL_RPATH_USE_LINK_PATH TRUE
)
bundle_GPISpace_add_rpath (TARGET SINGULAR-parallel-module INSTALL_DIRECTORY "LIB")

set(PFD_EXAMPLE_PATH "${CMAKE_INSTALL_PREFIX}/example_data")

add_subdirectory (interface)
add_subdirectory (workflow)
add_subdirectory (singular)
add_subdirectory (example_data)

configure_file("src/config.hpp.in"
  "${CMAKE_BINARY_DIR}/src/config.hpp"
  @ONLY
)

add_custom_target (Config
  ALL
  DEPENDS "${CMAKE_BINARY_DIR}/src/config.hpp"
)


#install (DIRECTORY "${SINGULAR_HOME}/"
#  DESTINATION "${BUNDLE_ROOT}/singular"
#  USE_SOURCE_PERMISSIONS
#  PATTERN "share/singular/html/*" EXCLUDE
#)
