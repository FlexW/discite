add_executable(editor)
set_warnings_as_errors(editor)

target_include_directories(editor PRIVATE .)

target_sources(editor PRIVATE
  main.cpp
  imgui.cpp
  editor_layer.cpp
  imgui_layer.cpp
  imgui_panel.cpp
  renderer_panel.cpp
  viewport_panel.cpp
  dockspace_panel.cpp
  scene_panel.cpp
  entity_panel.cpp
  )

target_link_libraries(editor PRIVATE
  imgui
  imguizmo
  game
  )
