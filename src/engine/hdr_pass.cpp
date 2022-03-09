#include "hdr_pass.hpp"
#include "gl_framebuffer.hpp"
#include <memory>

namespace dc
{

HdrPass::HdrPass()
{
  init_shaders();
  glGenVertexArrays(1, &quad_vertex_array_);
}

HdrPass::~HdrPass()
{
  if (quad_vertex_array_)
  {
    glDeleteVertexArrays(1, &quad_vertex_array_);
  }
}

void HdrPass::execute(const SceneRenderInfo         &scene_render_info,
                      const ViewRenderInfo          &view_render_info,
                      std::shared_ptr<GlFramebuffer> scene_framebuffer)
{
  const auto framebuffer = view_render_info.framebuffer();

  if (framebuffer.has_value())
  {
    // render in the user submitted framebuffer
    framebuffer.value()->bind();
  }
  else
  {
    // render to the windows default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  const auto viewport_info = view_render_info.viewport_info();
  glViewport(0, 0, viewport_info.width_, viewport_info.height_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  hdr_shader_->bind();

  const auto scene_tex = std::get<std::shared_ptr<GlTexture>>(
      scene_framebuffer->color_attachment(0));

  glActiveTexture(GL_TEXTURE0);
  scene_tex->bind();
  hdr_shader_->set_uniform("exposure", exposure_);
  hdr_shader_->set_uniform("hdr_tex", 0);

  // draw quad
  glBindVertexArray(quad_vertex_array_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  hdr_shader_->unbind();

  // bind default framebuffer in any case
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (output_)
  {
    output_(scene_render_info, view_render_info, scene_framebuffer);
  }
}

void HdrPass::set_output(Output output) { output_ = output; }

void HdrPass::init_shaders()
{
  hdr_shader_ = std::make_shared<GlShader>();
  hdr_shader_->init("shaders/quad.vert", "shaders/hdr.frag");
}

} // namespace dc
