#pragma once

#include "gl.hpp"
#include "gl_cube_texture.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"
#include "graphic/framebuffer.hpp"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace dc
{

class GlFramebuffer : public Framebuffer
{
public:
  GlFramebuffer(const FramebufferConfig &config);
  ~GlFramebuffer() override;

  void bind();
  void unbind();

  Attachment color_attachment(std::size_t index) const override;

  void set_color_attachment(std::size_t              index,
                            std::shared_ptr<Texture> texture) override;

  void set_color_attachment(std::size_t                  index,
                            std::shared_ptr<CubeTexture> texture,
                            int                          face,
                            unsigned                     mip = 0) override;

  Attachment depth_attachment() const override;

  void set_depth_attachment(std::shared_ptr<Renderbuffer> value) override;

  void set_depth_attachment(std::shared_ptr<CubeTexture> value) override;

  Attachment stencil_attachment() const override;

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

  void attach(const FramebufferConfig &config);
};

} // namespace dc
