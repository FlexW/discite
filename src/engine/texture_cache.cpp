#include "texture_cache.hpp"
#include "gl_texture.hpp"

#include <memory>

void TextureCache::set_import_path(const std::filesystem::path &value)
{
  import_path_ = value;
}

std::shared_ptr<GlTexture> TextureCache::get(const std::filesystem::path &name)
{
  const auto path = import_path_ / name;
  const auto iter = cache_.find(path.string());

  if (iter != cache_.end())
  {
    return iter->second;
  }

  auto texture = std::make_shared<GlTexture>();
  texture->load_from_file(path, true);
  cache_[path.string()] = texture;

  return texture;
}
