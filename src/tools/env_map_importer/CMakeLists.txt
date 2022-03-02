add_executable(env_map_importer)
set_warnings_as_errors(env_map_importer)

target_include_directories(env_map_importer PRIVATE .)

target_sources(env_map_importer PRIVATE
  main.cpp
  env_map_importer_layer.cpp
  )

target_link_libraries(env_map_importer PRIVATE
  game
  )
