set(HOME $ENV{HOME})
set(USER $ENV{USER})
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)

configure_file(pfd_gspc.lib.in pfd_gspc.lib)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/pfd_gspc.lib"
  DESTINATION "${PFD_SING_LIB_PATH}"
)

install (FILES "pfd.lib"
  DESTINATION "${PFD_SING_LIB_PATH}"
)

configure_file(pfd_gspc_interface.lib.in pfd_gspc_interface.lib)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/pfd_gspc_interface.lib"
  DESTINATION "${PFD_SING_LIB_PATH}"
)
