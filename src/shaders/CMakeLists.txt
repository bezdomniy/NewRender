set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB SLANG_SHADERS CONFIGURE_DEPENDS
    ${SHADER_DIR}/*.slang
)

set(SPIRV_BIN "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY ${SPIRV_BIN})

function(compile_shader shader)
    get_filename_component(FILE_NAME ${shader} NAME)
    set(SPIRV_OUTPUT "${SPIRV_BIN}/${FILE_NAME}.spv")


    cmake_print_variables(SPIRV_OUTPUT)

    add_custom_command(
        COMMENT
        "Transpiling shader '${shader}' into '${SHADER_OUTPUT}'..."
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND
        ${SLANGC}
        ${shader}
        -entry main
        -o ${SPIRV_OUTPUT}
        DEPENDS ${shader} ${SLANG_SHADERS}
        VERBATIM
    )

    list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
    set(SPIRV_SHADERS ${SPIRV_SHADERS} PARENT_SCOPE)

endfunction()

foreach(SHADER ${SLANG_SHADERS})
    compile_shader(${SHADER})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})
