add_library(engine)
set_warnings_as_errors(engine)

 # We include the GL loader ourselfs
target_compile_definitions(engine PUBLIC
  GLFW_INCLUDE_NONE
  SPDLOG_ACTIVE_LEVEL=0)

# Make the MSVC compilers preprocessor more standard like
if ((MSVC))
  target_compile_options(engine PUBLIC "/Zc:preprocessor")
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES RELEASE)
  target_compile_definitions(engine PUBLIC DC_ENABLE_ASSERT)
  # target_compile_definitions(engine PUBLIC DC_ENABLE_PROFILING)
  target_compile_definitions(engine PUBLIC DC_ENABLE_TIMING)
endif()

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
  string.cpp
  time.cpp
  material.cpp
  mesh.cpp
  engine.cpp
  directional_light.cpp
  point_light.cpp
  layer_stack.cpp
  gl_shader_storage_buffer.cpp
  gl_cube_texture.cpp
  asset.cpp
  mesh_asset.cpp
  env_map_asset.cpp
  material_asset.cpp
  texture_asset.cpp
  asset_handle.cpp
  asset_cache.cpp
  importer.cpp
  asset_importer_manager.cpp
  mesh_asset_importer.cpp
  cmd_args_parser.cpp
  serialization.cpp
  texture_asset.cpp
  environment_map.cpp
  uuid.cpp
  frame_data.cpp
  shadow_pass.cpp
  forward_pass.cpp
  skybox_pass.cpp
  hdr_pass.cpp
  render_object.cpp
  )

target_link_libraries(engine PUBLIC
  fastdelegate
  glm
  stb
  imgui
  glad
  glfw
  assimp
  gli
  rapidjson
  fmt
  spdlog
  OptickCore
  )

target_compile_features(engine PUBLIC cxx_std_17)
