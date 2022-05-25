#include "gl_renderbuffer.hpp"
#include "gl_helper.hpp"

namespace dc
{

GlRenderbuffer::GlRenderbuffer(const RenderbufferConfig &config)
{
  glCreateRenderbuffers(1, &id_);
  if (config.msaa_ == 0)
  {
    glNamedRenderbufferStorage(id_,
                               to_gl(config.sized_format_),
                               config.width_,
                               config.height_);
  }
  else
  {
    glNamedRenderbufferStorageMultisample(id_,
                                          config.msaa_,
                                          to_gl(config.sized_format_),
                                          config.width_,
                                          config.height_);
  }
}

GlRenderbuffer::~GlRenderbuffer() { glDeleteRenderbuffers(1, &id_); }

GLuint GlRenderbuffer::id() const { return id_; }

} // namespace dc
