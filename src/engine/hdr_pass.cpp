#include "hdr_pass.hpp"
#include "gl_framebuffer.hpp"

#include <array>
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

void HdrPass::execute(const SceneRenderInfo &        scene_render_info,
                      const ViewRenderInfo &         view_render_info,
                      std::shared_ptr<GlFramebuffer> scene_framebuffer,
                      std::shared_ptr<GlTexture>     bloom_texture)
{
  const auto framebuffer = view_render_info.framebuffer();

  GLuint framebuffer_id{0};
  if (framebuffer.has_value())
  {
    // render in the user submitted framebuffer
    framebuffer_id = framebuffer.value()->id();
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
  const auto viewport_info = view_render_info.viewport_info();
  glViewport(0, 0, viewport_info.width_, viewport_info.height_);
  constexpr std::array<float, 4> clear_color{0.0f, 0.0f, 0.0f, 1.0f};
  glClearNamedFramebufferfv(framebuffer_id, GL_COLOR, 0, clear_color.data());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  hdr_shader_->bind();

  const auto scene_tex = std::get<std::shared_ptr<GlTexture>>(
      scene_framebuffer->color_attachment(0));

  scene_tex->bind_unit(0);
  hdr_shader_->set_uniform("exposure", exposure_);
  hdr_shader_->set_uniform("hdr_tex", 0);

  hdr_shader_->set_uniform("is_bloom", bloom_texture != nullptr);
  if (bloom_texture)
  {
    bloom_texture->bind_unit(1);
    hdr_shader_->set_uniform("bloom_intensity", bloom_intensity_);
    hdr_shader_->set_uniform("bloom_tex", 1);
  }

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
