add_executable(discite)
set_warnings_as_errors(discite)

target_include_directories(discite PRIVATE .)

target_sources(discite PRIVATE
  main.cpp
  )

target_link_libraries(discite PRIVATE
  game
  )
