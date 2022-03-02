add_executable(scene_importer)
set_warnings_as_errors(scene_importer)

target_include_directories(scene_importer PRIVATE .)

target_sources(scene_importer PRIVATE
  main.cpp
  scene_importer_layer.cpp
  )

target_link_libraries(scene_importer PRIVATE
  game
  )
