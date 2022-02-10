add_subdirectory(engine)

add_executable(app)
set_warnings_as_errors(app)

target_sources(app PRIVATE
  main.cpp
  discite.cpp
  scene.cpp
  render_system.cpp
  camera_system.cpp
  camera_component.cpp
  renderer.cpp
  entity.cpp
  transform_component.cpp
  )

target_link_libraries(app PRIVATE
  engine
  EnTT::EnTT
  )
