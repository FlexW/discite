#pragma once

#include "gl_texture.hpp"

#include <unordered_map>

class TextureCache
{
public:
  void set_import_path(const std::filesystem::path &value);

  std::shared_ptr<GlTexture> get(const std::filesystem::path &name);

private:
  std::filesystem::path import_path_;

  std::unordered_map<std::string, std::shared_ptr<GlTexture>> cache_;
};
