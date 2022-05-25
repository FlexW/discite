#pragma once

#include "cube_texture.hpp"
#include "renderbuffer.hpp"
#include "texture.hpp"
#include "texture_array.hpp"

#include <memory>
#include <optional>
#include <variant>

namespace dc
{

enum class AttachmentType
{
  Texture,
  Renderbuffer,
};

struct FramebufferAttachmentCreateConfig
{
  AttachmentType     type_;
  TextureFormat      format_;
  TextureSizedFormat sized_format_;
  std::size_t        width_;
  std::size_t        height_;
  unsigned           msaa_{0};
};

using TextureArrayAttachment = std::shared_ptr<TextureArray>;
using TextureAttachment      = std::shared_ptr<Texture>;
using RenderbufferAttachment = std::shared_ptr<Renderbuffer>;
using CubeTextureAttachment  = std::shared_ptr<CubeTexture>;

using FramebufferAttachmentConfig =
    std::variant<TextureAttachment,
                 RenderbufferAttachment,
                 TextureArrayAttachment,
                 CubeTextureAttachment,
                 FramebufferAttachmentCreateConfig>;

struct FramebufferConfig
{
  std::vector<FramebufferAttachmentConfig>   color_attachments_;
  std::optional<FramebufferAttachmentConfig> depth_attachment_;
  std::optional<FramebufferAttachmentConfig> stencil_attachment_;
};

using Attachment = std::variant<TextureAttachment,
                                TextureArrayAttachment,
                                RenderbufferAttachment,
                                CubeTextureAttachment>;

class Framebuffer
{
public:
  virtual ~Framebuffer() = default;

  virtual Attachment color_attachment(std::size_t index) const = 0;

  virtual void set_color_attachment(std::size_t              index,
                                    std::shared_ptr<Texture> texture) = 0;

  virtual void set_color_attachment(std::size_t                  index,
                                    std::shared_ptr<CubeTexture> texture,
                                    int                          face,
                                    unsigned                     mip = 0) = 0;

  virtual Attachment depth_attachment() const = 0;

  virtual void set_depth_attachment(std::shared_ptr<Renderbuffer> value) = 0;

  virtual void set_depth_attachment(std::shared_ptr<CubeTexture> value) = 0;

  virtual Attachment stencil_attachment() const = 0;
};

} // namespace dc
