target_sources(game PRIVATE
  audio_engine.cpp
  open_al_helper.cpp
  audio_buffer.cpp
  audio_source.cpp
  audio_source_component.cpp
  audio_listener_component.cpp
  wav_file.cpp
  audio_asset.cpp
  audio_system.cpp
  )

# We use the static version of OpenAL
target_compile_definitions(game PUBLIC
  AL_LIBTYPE_STATIC)
