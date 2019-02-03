add_custom_target(shader_compilation ALL)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
function(compile_shader shaderJson shaderGLSL)
  set(outputGLSL ${CMAKE_BINARY_DIR}/shaders/${shaderGLSL})
  set(outputSPV ${CMAKE_BINARY_DIR}/bin/${shaderGLSL}.spv)
  set(inputJson ${CMAKE_CURRENT_SOURCE_DIR}/${shaderJson})
  set(inputGLSL ${CMAKE_CURRENT_SOURCE_DIR}/${shaderGLSL})
  add_custom_command(
    OUTPUT ${outputGLSL}
    VERBATIM
    MAIN_DEPENDENCY ${inputJson}
    COMMENT "Generating ${outputGLSL}"
    COMMAND ${CMAKE_BINARY_DIR}/bin/json-shader ${inputJson} ${inputGLSL} ${outputGLSL})
  add_custom_command(
    OUTPUT ${outputSPV}
    VERBATIM
    MAIN_DEPENDENCY ${outputGLSL}
    COMMENT "Generating ${outputSPV}"
    COMMAND glslangValidator -V ${outputGLSL} -o ${outputSPV})
  add_custom_target(${shaderGLSL}-gen DEPENDS ${outputSPV})
  add_dependencies(shader_compilation ${shaderGLSL}-gen)
endfunction()

compile_shader(triangle.vert.json triangle.vert)
compile_shader(triangle.frag.json triangle.frag)