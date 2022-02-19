add_library(engine)
set_warnings_as_errors(engine)

 # We include the GL loader ourselfs
target_compile_definitions(engine PUBLIC GLFW_INCLUDE_NONE)

target_include_directories(engine PUBLIC .)

target_sources(engine PRIVATE
  math.cpp
  event.cpp
  event_manager.cpp
  window.cpp
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
  engine.cpp
  directional_light.cpp
  point_light.cpp
  layer_stack.cpp
  )

target_link_libraries(engine PUBLIC
  fastdelegate
  glm
  stb
  imgui
  glad
  glfw
  assimp
  )

target_compile_features(engine PUBLIC cxx_std_17)
