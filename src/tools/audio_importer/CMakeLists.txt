add_executable(audio_importer)
set_warnings_as_errors(audio_importer)

target_include_directories(audio_importer PRIVATE .)

target_sources(audio_importer PRIVATE
  main.cpp
  audio_importer_layer.cpp
  )

target_link_libraries(audio_importer PRIVATE
  game
  )
