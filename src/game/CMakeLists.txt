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
  model_component.cpp
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
  )

target_link_libraries(game PUBLIC
  engine
  EnTT::EnTT
  ${MONO_LIBRARIES}
  )

target_include_directories(game SYSTEM PUBLIC
  ${MONO_INCLUDE_DIRS}
  )

add_subdirectory(script)
