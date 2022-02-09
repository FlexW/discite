add_subdirectory(engine)

add_executable(app)
set_warnings_as_errors(app)

target_sources(app PRIVATE
  main.cpp
  game.cpp
  )

target_link_libraries(app PRIVATE
  engine
  )
