add_library(stb STATIC)
target_sources(stb PRIVATE
  stb_image.c
  stb_image_write.c
  stb_image_resize.c
  )
target_include_directories(stb PUBLIC .)
