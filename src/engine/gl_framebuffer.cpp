#include "gl_framebuffer.hpp"
#include "assert.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_texture.hpp"
#include "gl_texture_array.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>

namespace dc
{

GlFramebuffer::GlFramebuffer() { glCreateFramebuffers(1, &id_); }

GlFramebuffer::~GlFramebuffer() { glDeleteFramebuffers(1, &id_); }

void GlFramebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, id_); }

void GlFramebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void GlFramebuffer::attach(const FramebufferConfig &config)
{
  // Attach the color attachments
  DC_ASSERT(config.color_attachments_.size() <= GL_MAX_COLOR_ATTACHMENTS,
            "Too much color attachments");
  std::vector<GLuint> color_attachment_targets;
  for (std::size_t i = 0;
       i < config.color_attachments_.size() && i < GL_MAX_COLOR_ATTACHMENTS;
       ++i)
  {
    const auto &attachment_config = config.color_attachments_[i];
    if (std::holds_alternative<FramebufferAttachmentCreateConfig>(
            attachment_config))
    {
      const auto &color_attachment =
          std::get<FramebufferAttachmentCreateConfig>(attachment_config);

      switch (color_attachment.type_)
      {
      case AttachmentType::Texture:
      {
        GlTextureConfig texture_config{};
        texture_config.width_        = color_attachment.width_;
        texture_config.height_       = color_attachment.height_;
        texture_config.sized_format_ = color_attachment.internal_format_;
        texture_config.wrap_s_       = GL_REPEAT;
        texture_config.wrap_t_       = GL_REPEAT;
        texture_config.min_filter_   = GL_LINEAR;
        texture_config.mag_filter_   = GL_LINEAR;
        texture_config.msaa_         = color_attachment.msaa_;

        auto       texture = std::make_shared<GlTexture>(texture_config);
        const auto target  = GL_COLOR_ATTACHMENT0 + i;
        glNamedFramebufferTexture(id_, target, texture->id(), 0);
        color_attachment_targets.push_back(target);
        color_attachments_.push_back(std::move(texture));
        break;
      }
      case AttachmentType::Renderbuffer:
      {
        GlRenderbufferConfig renderbuffer_config{};
        renderbuffer_config.sized_format_ = color_attachment.internal_format_;
        renderbuffer_config.width_        = color_attachment.width_;
        renderbuffer_config.height_       = color_attachment.height_;
        renderbuffer_config.msaa_         = color_attachment.msaa_;
        auto renderbuffer    = std::make_shared<GlRenderbuffer>(renderbuffer_config);

        const auto target = GL_COLOR_ATTACHMENT0 + i;
        glNamedFramebufferRenderbuffer(id_,
                                       target,
                                       GL_RENDERBUFFER,
                                       renderbuffer->id());

        color_attachment_targets.push_back(target);
        color_attachments_.push_back(std::move(renderbuffer));
        break;
      }
      }
    }
    else if (std::holds_alternative<std::shared_ptr<GlTextureArray>>(
                 attachment_config))
    {
      const auto tex_array =
          std::get<std::shared_ptr<GlTextureArray>>(attachment_config);
      glNamedFramebufferTexture(id_,
                                GL_COLOR_ATTACHMENT0 + i,
                                tex_array->id(),
                                0);
      color_attachments_.push_back(tex_array);
    }
    else
    {
      DC_FAIL("Not implemented");
    }
  }

  if (color_attachment_targets.size() == 0)
  {
    glNamedFramebufferDrawBuffer(id_, GL_NONE);
    glNamedFramebufferReadBuffer(id_, GL_NONE);
  }
  else
  {
    glNamedFramebufferDrawBuffers(id_,
                                  color_attachment_targets.size(),
                                  color_attachment_targets.data());
  }

  // Attach the depth attachment
  if (config.depth_attachment_.has_value())
  {
    const auto &attachment_config = config.depth_attachment_.value();
    if (std::holds_alternative<FramebufferAttachmentCreateConfig>(
            attachment_config))
    {
      const auto &depth_attachment =
          std::get<FramebufferAttachmentCreateConfig>(attachment_config);
      switch (depth_attachment.type_)
      {
      case AttachmentType::Texture:
      {
        GlTextureConfig texture_config{};
        texture_config.width_        = depth_attachment.width_;
        texture_config.height_       = depth_attachment.height_;
        texture_config.sized_format_ = depth_attachment.internal_format_;
        texture_config.wrap_s_       = GL_REPEAT;
        texture_config.wrap_t_       = GL_REPEAT;
        texture_config.min_filter_   = GL_LINEAR;
        texture_config.mag_filter_   = GL_LINEAR;
        texture_config.msaa_         = depth_attachment.msaa_;

        auto texture         = std::make_shared<GlTexture>(texture_config);
        glNamedFramebufferTexture(id_, GL_DEPTH_ATTACHMENT, texture->id(), 0);
        color_attachments_.push_back(std::move(texture));
        break;
      }
      case AttachmentType::Renderbuffer:
      {
        GlRenderbufferConfig renderbuffer_config{};
        renderbuffer_config.sized_format_ = depth_attachment.internal_format_;
        renderbuffer_config.width_        = depth_attachment.width_;
        renderbuffer_config.height_       = depth_attachment.height_;
        renderbuffer_config.msaa_         = depth_attachment.msaa_;
        auto renderbuffer    = std::make_shared<GlRenderbuffer>(renderbuffer_config);

        glNamedFramebufferRenderbuffer(id_,
                                       GL_DEPTH_ATTACHMENT,
                                       GL_RENDERBUFFER,
                                       renderbuffer->id());
        depth_attachment_ = std::move(renderbuffer);
        break;
      }
      }
    }
    else if (std::holds_alternative<std::shared_ptr<GlTextureArray>>(
                 attachment_config))
    {
      const auto tex_array =
          std::get<std::shared_ptr<GlTextureArray>>(attachment_config);
      glNamedFramebufferTexture(id_, GL_DEPTH_ATTACHMENT, tex_array->id(), 0);
      depth_attachment_ = tex_array;
    }
    else
    {
      DC_FAIL("Not implemented");
    }
  }

  // Attach the stencil attachment
  if (config.stencil_attachment_.has_value())
  {
    const auto &attachment_config = config.depth_attachment_.value();
    if (std::holds_alternative<FramebufferAttachmentCreateConfig>(
            attachment_config))
    {
      const auto &stencil_attachment =
          std::get<FramebufferAttachmentCreateConfig>(attachment_config);
      switch (stencil_attachment.type_)
      {
      case AttachmentType::Texture:
      {
        DC_FAIL("Not implemented");
        break;
      }
      case AttachmentType::Renderbuffer:
      {
        GlRenderbufferConfig renderbuffer_config{};
        renderbuffer_config.sized_format_ = stencil_attachment.internal_format_;
        renderbuffer_config.width_        = stencil_attachment.width_;
        renderbuffer_config.height_       = stencil_attachment.height_;
        renderbuffer_config.msaa_         = stencil_attachment.msaa_;

        auto renderbuffer = std::make_shared<GlRenderbuffer>(renderbuffer_config);

        glNamedFramebufferRenderbuffer(id_,
                                       GL_STENCIL_ATTACHMENT,
                                       GL_RENDERBUFFER,
                                       renderbuffer->id());

        stencil_attachment_ = std::move(renderbuffer);
        break;
      }
      }
    }
    else
    {
      DC_FAIL("Not implemented");
    }
  }

  // Check if the framebuffer is complete
  const auto result = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
  switch (result)
  {
  case GL_FRAMEBUFFER_COMPLETE:
  {
    // Nothing to do, all fine.
    break;
  }
  case GL_FRAMEBUFFER_UNDEFINED:
  {
    throw std::runtime_error("Bound framebuffer is undefined");
  }
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
  {
    throw std::runtime_error(
        "A necessary attachment to the framebuffer is uninitialized");
  }
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
  {
    throw std::runtime_error(
        "A necessary attachment to the framebuffer is missing");
  }
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
  {
    throw std::runtime_error("Incomplete draw buffer");
  }
  case GL_FRAMEBUFFER_UNSUPPORTED:
  {
    throw std::runtime_error("Combination of attachments is unsupported");
  }
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
  {
    throw std::runtime_error(
        "Number of samples for all images across framebuffer do not match");
  }
  default:
  {
    throw std::runtime_error("Unknown framebuffer error");
  }
  };
}

Attachment GlFramebuffer::color_attachment(std::size_t index) const
{
  if (index >= color_attachments_.size())
  {
    return {};
  }

  return color_attachments_[index];
}

Attachment GlFramebuffer::depth_attachment() const { return depth_attachment_; }

Attachment GlFramebuffer::stencil_attachment() const
{
  return stencil_attachment_;
}

GLuint GlFramebuffer::id() const { return id_; }

} // namespace dc
