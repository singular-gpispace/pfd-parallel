set(HOME $ENV{HOME})
set(USER $ENV{USER})
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)

configure_file(pfd_gspc.lib.in pfd_gspc.lib)
configure_file(pfd_singular_interface.lib.in pfd_singular_interface.lib)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/pfd_singular_interface.lib"
               "${CMAKE_CURRENT_BINARY_DIR}/pfd_gspc.lib"
               "pfd.lib"
  DESTINATION "${PFD_SING_LIB_PATH}"
)
