add_executable(brdf_lut)
set_warnings_as_errors(brdf_lut)

target_include_directories(brdf_lut PRIVATE .)

target_sources(brdf_lut PRIVATE
  main.cpp
  brdf_lut_layer.cpp
  )

target_link_libraries(brdf_lut PRIVATE
  game
  )
