add_executable(irr_map)
set_warnings_as_errors(irr_map)

target_include_directories(irr_map PRIVATE .)

target_sources(irr_map PRIVATE
  main.cpp
  irr_map_layer.cpp
  )

target_link_libraries(irr_map PRIVATE
  game
  )
