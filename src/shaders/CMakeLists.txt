function(compile_shader shaderJson shaderGLSL)
  add_custom_command(
    OUTPUT ${shaderGLSL}
    MAIN_DEPENDENCY ${shaderJson}
    COMMENT "Generating ${shaderGLSL}"
    COMMAND json-shader -o ${shaderGLSL} ${shaderJson})
endfunction()

compile_shader(triangle.vert.json triangle.vert.spv)