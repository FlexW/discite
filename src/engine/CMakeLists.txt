add_library(engine)
set_warnings_as_errors(engine)

target_sources(engine PRIVATE
  camera.cpp
  config.cpp
  debug_draw.cpp
  event.cpp
  event_manager.cpp
  gl_framebuffer.cpp
  gl_index_buffer.cpp
  gl_renderbuffer.cpp
  gl_shader.cpp
  gl_texture.cpp
  gl_texture_array.cpp
  gl_vertex_buffer.cpp
  gl_vertex_array.cpp
  gl.cpp
  image.cpp
  log.cpp
  string.cpp
  time.cpp
  material.cpp
  texture_cache.cpp
  mesh.cpp
  application.cpp
  directional_light.cpp
  )

target_link_libraries(engine PUBLIC
  fastdelegate
  glm
  stb
  glad
  glfw
  assimp
  )

target_compile_features(engine PUBLIC cxx_std_17)
