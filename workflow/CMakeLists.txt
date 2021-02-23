# Note: Does _NOT_ track external dependencies!

function(add_petrinet in out module_name)
  add_custom_command (
    OUTPUT "${CMAKE_BINARY_DIR}/parallel_${module_name}.pnet"
           "${CMAKE_BINARY_DIR}/gen_${out}/pnetc/op/libsingular_parallel_${module_name}.so"
    COMMAND "${GSPC_HOME}/bin/pnetc"
            --gen-cxxflags=-O3
            --gen-cxxflags=-std=c++17
            --gen-cxxflags=-I"${CMAKE_BINARY_DIR}"
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}"
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}/src"
            --gen-cxxflags=-I"${GSPC_HOME}/include"
            --gen-cxxflags=-I"${GSPC_HOME}/external/boost/include"
            --gen-ldflags="-L${CMAKE_BINARY_DIR}"
            --gen-ldflags="-L${CMAKE_BINARY_DIR}/src/util-generic"
            --gen-ldflags="-Wl,-rpath='${GSPC_HOME}/lib'"
            --gen-ldflags="-Wl,-rpath='${GSPC_HOME}/external/boost/lib'"
            --force-overwrite-file=true
            --input="${CMAKE_SOURCE_DIR}/workflow/${in}"
            --output="${CMAKE_BINARY_DIR}/${out}"
            --path-to-cpp="${CMAKE_BINARY_DIR}/gen_${out}"
            --Wbackup-file=OFF
            --Woverwrite-file=OFF
            &&
            make -C "${CMAKE_BINARY_DIR}/gen_${out}"
    DEPENDS "${in}"
  )
  add_custom_target ("${out}_and_wrapper-library"
    ALL
    DEPENDS "${CMAKE_BINARY_DIR}/gen_${out}/pnetc/op/libsingular_parallel_${module_name}.so"
            "${CMAKE_BINARY_DIR}/${out}"
  )

endfunction()

add_petrinet(parallel_all.xpnet parallel_all.pnet all)
add_petrinet(parallel_first.xpnet parallel_first.pnet first)

install (FILES
  "${CMAKE_BINARY_DIR}/parallel_all.pnet"
  "${CMAKE_BINARY_DIR}/parallel_first.pnet"
  DESTINATION libexec/workflow
)

install (FILES
  "${CMAKE_BINARY_DIR}/gen_parallel_all.pnet/pnetc/op/libsingular_parallel_all.so"
  "${CMAKE_BINARY_DIR}/gen_parallel_first.pnet/pnetc/op/libsingular_parallel_first.so"
  DESTINATION libexec/workflow
)
