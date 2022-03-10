#pragma once

#include "gl.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace dc
{

enum class AttachmentType
{
  Texture,
  Renderbuffer,
};

struct FramebufferAttachmentCreateConfig
{
  AttachmentType type_;
  GLenum         format_;
  GLint          internal_format_;
  GLsizei        width_;
  GLsizei        height_;
};

using FramebufferAttachmentConfig =
    std::variant<std::shared_ptr<GlTexture>,
                 std::shared_ptr<GlRenderbuffer>,
                 std::shared_ptr<GlTextureArray>,
                 FramebufferAttachmentCreateConfig>;

struct FramebufferConfig
{
  std::vector<FramebufferAttachmentConfig>   color_attachments_;
  std::optional<FramebufferAttachmentConfig> depth_attachment_;
  std::optional<FramebufferAttachmentConfig> stencil_attachment_;
};

using TextureArrayAttachment = std::shared_ptr<GlTextureArray>;
using TextureAttachment      = std::shared_ptr<GlTexture>;
using RenderbufferAttachment = std::shared_ptr<GlRenderbuffer>;
using Attachment             = std::
    variant<TextureAttachment, TextureArrayAttachment, RenderbufferAttachment>;

class GlFramebuffer
{
public:
  GlFramebuffer();
  ~GlFramebuffer();

  void bind();
  void unbind();

  void attach(const FramebufferConfig &config);

  Attachment color_attachment(std::size_t index) const;
  Attachment depth_attachment() const;
  Attachment stencil_attachment() const;

  GLuint id() const;

private:
  GLuint id_{};

  std::vector<Attachment> color_attachments_;
  Attachment              depth_attachment_;
  Attachment              stencil_attachment_;

  GlFramebuffer(const GlFramebuffer &other) = delete;
  void operator=(const GlFramebuffer &other) = delete;
  GlFramebuffer(GlFramebuffer &&other)       = delete;
  void operator=(GlFramebuffer &&other) = delete;
};

} // namespace dc
