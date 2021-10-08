#list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

#find_package (Singular REQUIRED)

extended_add_library (
  NAME parallel_implementation
  TYPE MODULE
  SOURCES "singular_pnet-interface.cpp"
          "singular_functions.cpp"
          "pnet_logging.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
  COMPILE_DEFINITIONS SING_NDEBUG OM_NDEBUG
  LIBRARIES GPISpace::workflow_development
            partial_static_parallel_allpfd_wrapper
            partial_static_parallel_all_wrapper
            partial_static_parallel_first_wrapper
            Singular
  DEPENDS parallel_all_and_wrapper-library
          parallel_first_and_wrapper-library
          parallel_allpfd_and_wrapper-library
          Config
  INSTALL INSTALL_DESTINATION "libexec/workflow"
)
set_target_properties (parallel_implementation
  PROPERTIES
  INSTALL_RPATH_USE_LINK_PATH TRUE
)

#install (FILES LIB/parallelgspc.lib
#  DESTINATION "${CMAKE_INSTALL_PREFIX}"
#)
