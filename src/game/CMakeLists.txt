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
  )

target_link_libraries(game PUBLIC
  engine
  EnTT::EnTT
  )
