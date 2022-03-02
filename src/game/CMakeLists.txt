add_library(game)
set_warnings_as_errors(game)

target_include_directories(game PUBLIC .)

target_sources(game PRIVATE
  game_layer.cpp
  scene.cpp
  render_system.cpp
  camera_system.cpp
  camera_component.cpp
  renderer.cpp
  entity.cpp
  transform_component.cpp
  sky.cpp
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
  )

target_link_libraries(game PUBLIC
  engine
  EnTT::EnTT
  )
