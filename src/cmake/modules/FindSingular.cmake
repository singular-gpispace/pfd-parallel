# hints SINGULAR_HOME, GMP_HOME
# library Singular

find_path (SINGULAR_HOME
  NAMES "lib/libSingular.so" "include/singular/Singular/libsingular.h"
  HINTS ${SINGULAR_HOME} ENV SINGULAR_HOME
)

find_library (SINGULAR_LIBRARY
  NAMES "Singular"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "lib"
)

find_library (OMALLOC_LIBRARY
  NAMES "omalloc"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "lib"
)

find_library (POLYS_LIBRARY
  NAMES "polys"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "lib"
)

find_library (RESOURCES_LIBRARY
  NAMES "singular_resources"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "lib"
)

find_path (SINGULAR_INCLUDE_DIR
  NAMES "singular/Singular/libsingular.h"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "include"
)

find_file (SINGULAR_CONFIG_BIN
  NAMES "libsingular-config"
  HINTS ${SINGULAR_HOME}
  PATH_SUFFIXES "bin"
)

if (SINGULAR_CONFIG_BIN)
  execute_process (
    COMMAND "${SINGULAR_CONFIG_BIN}" --version
             OUTPUT_VARIABLE SINGULAR_VERSION
             ERROR_VARIABLE _err
             RESULT_VARIABLE _res
  )
  if (NOT ${_res} EQUAL 0)
    message (FATAL_ERROR "could not discover revision info: ${_err}")
  endif()

  string (STRIP "${SINGULAR_VERSION}" SINGULAR_VERSION)
endif()

#! \note Singular includes gmp.h in one of the headers but doesn't
#!       bundle it.
find_path (GMP_INCLUDE_DIR
  NAMES gmp.h
  PATH_SUFFIXES "include"
  HINTS ${GMP_HOME} ENV GMP_HOME
)

#find_path (FLINT_HOME
#  NAMES "lib/libflint.so"
#  HINTS ${FLINT_HOME} ENV FLINT_HOME
#
#)

#find_library (FLINT_LIBRARY
#  NAMES "flint"
#  HINTS ${FLINT_HOME}
#  PATH_SUFFIXES "lib"
#)

find_library (GMP_LIBRARY
  NAMES "gmp"
  HINTS ${GMP_HOME} ENV GMP_HOME
)

mark_as_advanced ( SINGULAR_HOME
                   SINGULAR_LIBRARY
                   OMALLOC_LIBRARY
                   POLYS_LIBRARY
                   RESOURCES_LIBRARY
                   SINGULAR_INCLUDE_DIR
                   SINGULAR_CONFIG_BIN
                   GMP_INCLUDE_DIR
                   GMP_LIBRARY
#                  FLINT_HOME
#                  FLINT_LIBRARY
                   SINGULAR_VERSION
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Singular
  REQUIRED_VARS SINGULAR_HOME
                SINGULAR_LIBRARY
                OMALLOC_LIBRARY
                POLYS_LIBRARY
                RESOURCES_LIBRARY
                SINGULAR_INCLUDE_DIR
                SINGULAR_CONFIG_BIN
                GMP_INCLUDE_DIR
                GMP_LIBRARY
#               FLINT_HOME
#               FLINT_LIBRARY
  VERSION_VAR SINGULAR_VERSION
)

if (Singular_FOUND)
  extended_add_library (NAME Singular
    SYSTEM_INCLUDE_DIRECTORIES INTERFACE
      "${SINGULAR_INCLUDE_DIR}"
       #! \todo fix singular -> open issue there "please use one root only"
      "${SINGULAR_INCLUDE_DIR}/singular"
      "${GMP_INCLUDE_DIR}"
    LIBRARIES ${SINGULAR_LIBRARY}
#             ${FLINT_LIBRARY}
              ${OMALLOC_LIBRARY}
              ${POLYS_LIBRARY}
              ${RESOURCES_LIBRARY}
              ${GMP_LIBRARY}
  )
endif()

#! \todo bundling
