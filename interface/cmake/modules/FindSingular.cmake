# hints SINGULAR_HOME

# library Singular
# variable SINGULAR_HOME

find_path (SINGULAR_HOME
  NAMES "lib/libSingular.so" "include/singular/Singular/libsingular.h"
  HINTS ${SINGULAR_HOME} ENV SINGULAR_HOME
)

find_library (SINGULAR_LIBRARY
  NAMES "libSingular.so"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "lib"
)

if (SINGULAR_HOME)
  set (SINGULAR_INCLUDE_DIR "${SINGULAR_HOME}/include/")

  execute_process (
    COMMAND "${SINGULAR_HOME}/bin/libsingular-config" --version
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
             OUTPUT_VARIABLE _out
             ERROR_VARIABLE _err
             RESULT_VARIABLE _res
  )
  if (${_res} EQUAL 0)
    string (STRIP "${_out}" _out)
    set (SINGULAR_REVISION "${_out}")
  else()
    message (FATAL_ERROR "could not discover revision info")
  endif()
endif()

mark_as_advanced (SINGULAR_HOME
  SINGULAR_INCLUDE_DIR
  SINGULAR_LIBRARY
  SINGULAR_REVISION
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Singular
  REQUIRED_VARS SINGULAR_HOME
                SINGULAR_LIBRARY
                SINGULAR_INCLUDE_DIR
  VERSION_VAR SINGULAR_REVISION
)

extended_add_library (NAME Singular
  SYSTEM_INCLUDE_DIRECTORIES INTERFACE
    "${SINGULAR_INCLUDE_DIR}"
     #! \todo fix singular -> open issue there "please use one root only"
    "${SINGULAR_INCLUDE_DIR}/singular"
  LIBRARIES ${SINGULAR_LIBRARY}
)

#! \todo bundling
