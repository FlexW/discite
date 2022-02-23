#pragma once

#include "gl.hpp"

#include <filesystem>

class GlCubeTexture
{
public:
  GlCubeTexture(const std::filesystem::path &file_path);
  ~GlCubeTexture();

  void bind();
  void unbind();

private:
  GLuint id_{};
};
