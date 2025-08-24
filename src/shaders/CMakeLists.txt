set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(SPIRV_BIN "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY ${SPIRV_BIN})

function(compile_shader shader entry_point)
    get_filename_component(FILE_NAME ${shader} NAME)
    # set(SPIRV_OUTPUT "${SPIRV_BIN}/${FILE_NAME}.spv")
    set(SPIRV_OUTPUT "${SPIRV_BIN}/${FILE_NAME}.${entry_point}.spv")
    cmake_print_variables(SPIRV_OUTPUT)

    add_custom_command(
        COMMENT
        "Transpiling shader '${shader}' into '${SPIRV_OUTPUT}'..."
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND
        ${SLANGC}
        ${shader}
        -entry ${entry_point}
        -o ${SPIRV_OUTPUT}
        DEPENDS ${shader} 
        VERBATIM
    )

    list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
    set(SPIRV_SHADERS ${SPIRV_SHADERS} PARENT_SCOPE)

endfunction()

compile_shader(${SHADER_DIR}/hello-world.slang main)
compile_shader(${SHADER_DIR}/graphics.slang vertMain)
compile_shader(${SHADER_DIR}/graphics.slang fragMain)

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})

