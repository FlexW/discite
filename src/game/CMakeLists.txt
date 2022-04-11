find_package(Mono REQUIRED)

add_library(game)
set_warnings_as_errors(game)

target_include_directories(game PUBLIC .)

target_sources(game PRIVATE
  game_layer.cpp
  scene.cpp
  scene_events.cpp
  render_system.cpp
  camera_system.cpp
  camera_component.cpp
  scene_renderer.cpp
  entity.cpp
  transform_component.cpp
  scene_asset_importer.cpp
  mesh_component.cpp
  skinned_mesh_component.cpp
  guid_component.cpp
  name_component.cpp
  relationship_component.cpp
  point_light_component.cpp
  directional_light_component.cpp
  sky_component.cpp
  scene_asset.cpp
  guid_component.cpp
  systems_context.cpp
  scene_manager.cpp
  animation_system.cpp
  )

target_link_libraries(game PUBLIC
  engine
  EnTT::EnTT
  ${MONO_LIBRARIES}
  )

target_include_directories(game SYSTEM PUBLIC
  ${MONO_INCLUDE_DIRS}
  )

# Generate script runtime
file(GLOB SCRIPT_RUNTIME_FILES ${CMAKE_SOURCE_DIR}/script_runtime/*.cs)

add_custom_command(OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DisciteRuntime.dll
    COMMENT "Generate script runtime"
    COMMAND msbuild ${CMAKE_SOURCE_DIR}/script_runtime/DisciteRuntime.csproj
    DEPENDS ${SCRIPT_RUNTIME_FILES}
)
# Need to create a custom target for generating script runtime
add_custom_target(script_runtime ALL
  DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DisciteRuntime.dll
)


add_subdirectory(script)
add_subdirectory(physic)
add_subdirectory(audio)
