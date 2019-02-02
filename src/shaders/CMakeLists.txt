add_custom_target(shader_compilation)

function(compile_shader shaderJson shaderGLSL)
message("in shaders listfile: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}, CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}, CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("calling compile_shader with arguments ${shaderJson} and ${shaderGLSL}")
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${shaderGLSL}
    VERBATIM
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${shaderJson}
    COMMENT "Generating ${shaderGLSL}"
    COMMAND json-shader -o ${CMAKE_CURRENT_BINARY_DIR}/${shaderGLSL} ${CMAKE_CURRENT_SOURCE_DIR}/${shaderJson})
  add_dependencies()
endfunction()

compile_shader(triangle.vert.json triangle.vert)