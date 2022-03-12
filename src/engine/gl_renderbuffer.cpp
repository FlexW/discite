#include "gl_renderbuffer.hpp"

namespace dc
{

GlRenderbuffer::GlRenderbuffer(const GlRenderbufferConfig &config)
{
  glCreateRenderbuffers(1, &id_);
  if (config.msaa_ == 0)
  {
    glNamedRenderbufferStorage(id_,
                               config.sized_format_,
                               config.width_,
                               config.height_);
  }
  else
  {
    glNamedRenderbufferStorageMultisample(id_,
                                          config.msaa_,
                                          config.sized_format_,
                                          config.width_,
                                          config.height_);
  }
}

GlRenderbuffer::~GlRenderbuffer() { glDeleteRenderbuffers(1, &id_); }

GLuint GlRenderbuffer::id() const { return id_; }

} // namespace dc
