add_executable(mesh_importer)
set_warnings_as_errors(mesh_importer)

target_include_directories(mesh_importer PRIVATE .)

target_sources(mesh_importer PRIVATE
  main.cpp
  mesh_importer_layer.cpp
  )

target_link_libraries(mesh_importer PRIVATE
  game
  )
