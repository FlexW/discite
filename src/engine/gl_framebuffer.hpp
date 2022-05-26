#pragma once

#include "gl.hpp"
#include "gl_cube_texture.hpp"
#include "gl_cube_texture_array.hpp"
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
  GLuint         msaa_{0};
};

using TextureArrayAttachment = std::shared_ptr<GlTextureArray>;
using TextureAttachment      = std::shared_ptr<GlTexture>;
using RenderbufferAttachment = std::shared_ptr<GlRenderbuffer>;
using CubeTextureAttachment      = std::shared_ptr<GlCubeTexture>;
using CubeTextureArrayAttachment = std::shared_ptr<GlCubeTextureArray>;

using FramebufferAttachmentConfig =
    std::variant<TextureAttachment,
                 RenderbufferAttachment,
                 TextureArrayAttachment,
                 CubeTextureAttachment,
                 CubeTextureArrayAttachment,
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
                                CubeTextureAttachment,
                                CubeTextureArrayAttachment>;

class GlFramebuffer
{
public:
  GlFramebuffer();
  ~GlFramebuffer();

  void bind();
  void unbind();

  void attach(const FramebufferConfig &config);

  Attachment color_attachment(std::size_t index) const;
  void       set_color_attachment(std::size_t                index,
                                  std::shared_ptr<GlTexture> texture);
  void       set_color_attachment(std::size_t                    index,
                                  std::shared_ptr<GlCubeTexture> texture,
                                  int                            face,
                                  GLuint                         mip = 0);

  Attachment depth_attachment() const;
  void       set_depth_attachment(std::shared_ptr<GlRenderbuffer> value);
  void       set_depth_attachment(std::shared_ptr<GlCubeTexture> value);
  void       set_depth_attachment(std::shared_ptr<GlCubeTextureArray> value,
                                  GLint                               layer);
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
