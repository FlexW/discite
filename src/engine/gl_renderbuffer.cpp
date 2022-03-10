#include "gl_renderbuffer.hpp"

namespace dc
{

GlRenderbuffer::GlRenderbuffer(GLenum  internal_format,
                               GLsizei width,
                               GLsizei height)
{
  glCreateRenderbuffers(1, &id_);
  glNamedRenderbufferStorage(id_, internal_format, width, height);
}

GlRenderbuffer::GlRenderbuffer(GLenum  internal_format,
                               GLsizei samples,
                               GLsizei width,
                               GLsizei height)
{
  glCreateRenderbuffers(1, &id_);
  glNamedRenderbufferStorageMultisample(id_,
                                        samples,
                                        internal_format,
                                        width,
                                        height);
}

GlRenderbuffer::~GlRenderbuffer() { glDeleteRenderbuffers(1, &id_); }

GLuint GlRenderbuffer::id() const { return id_; }

} // namespace dc
