list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package (Singular REQUIRED)

extended_add_library (NAME parallel_implementation
  TYPE MODULE
  SOURCES "implementation.cpp" "singular_functions.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/gen"
  COMPILE_DEFINITIONS SING_NDEBUG OM_NDEBUG
  LIBRARIES GPISpace::workflow_development Singular
  DEPENDS parallel_all_and_wrapper-library
          parallel_first_and_wrapper-library
          parallel_allpfd_and_wrapper-library
  INSTALL INSTALL_DESTINATION "libexec/workflow"
)
set_target_properties (parallel_implementation
  PROPERTIES
  INSTALL_RPATH_USE_LINK_PATH TRUE
)

#install (FILES LIB/parallelgspc.lib
#  DESTINATION "${CMAKE_INSTALL_PREFIX}"
#)
