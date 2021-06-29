# Note: Does _NOT_ track external dependencies!
function(add_petrinet in out module_name)
  add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.pnet"
           "${CMAKE_CURRENT_BINARY_DIR}/gen_${module_name}/pnetc/op/libsingular_${module_name}.so"
    COMMAND "${GSPC_HOME}/bin/pnetc"
            --gen-cxxflags=-O3
            --gen-cxxflags=-std=c++17
            --gen-cxxflags=-I"${CMAKE_BINARY_DIR}"
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}"
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}/src"
            --gen-cxxflags=-I"${GPISPACE_REPO}/src"
            --gen-cxxflags=-I"${GSPC_HOME}/include"
            --gen-cxxflags=-I"${GSPC_HOME}/external/boost/include"
            --gen-ldflags="-L${CMAKE_BINARY_DIR}"
            --gen-ldflags="-L${CMAKE_BINARY_DIR}/src/util-generic"
            --gen-ldflags="-Wl,-rpath='${GSPC_HOME}/lib'"
            --gen-ldflags="-Wl,-rpath='${GSPC_HOME}/external/boost/lib'"
            --force-overwrite-file=true
            --input="${CMAKE_CURRENT_SOURCE_DIR}/${in}"
            --output="${CMAKE_CURRENT_BINARY_DIR}/${out}"
            --path-to-cpp="${CMAKE_CURRENT_BINARY_DIR}/gen_${module_name}"
            --Wbackup-file=OFF
            --Woverwrite-file=OFF
            &&
            make -C "${CMAKE_CURRENT_BINARY_DIR}/gen_${module_name}"
    DEPENDS "${in}"
            ${ARGN}
  )
  add_custom_target ("${module_name}_and_wrapper-library"
    ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/gen_${module_name}/pnetc/op/libsingular_${module_name}.so"
            "${CMAKE_CURRENT_BINARY_DIR}/${out}"
  )
endfunction()

function(addinstall_pnet module_name)
  # note, optional arguments are assumed to be dependencies
  add_petrinet(${module_name}.xpnet
               ${module_name}.pnet
               ${module_name}
               types.xpnet
               ${ARGN}
               )
  install (FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.pnet"
    DESTINATION libexec/workflow
  )
  install (FILES
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${module_name}/pnetc/op/libsingular_${module_name}.so"
    DESTINATION libexec/workflow
  )
endfunction()

addinstall_pnet(parallel_all)
addinstall_pnet(parallel_first)
addinstall_pnet(parallel_allpfd pfd_compute.xpnet)

