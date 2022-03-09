#include "skybox_pass.hpp"
#include "gl_framebuffer.hpp"
#include <memory>

namespace dc
{
SkyboxPass::SkyboxPass() { init_shaders(); }

SkyboxPass::~SkyboxPass()
{
  if (cube_vertex_array_)
  {
    glDeleteVertexArrays(1, &cube_vertex_array_);
  }

  if (cube_vertex_buffer_)
  {
    glDeleteBuffers(1, &cube_vertex_buffer_);
  }
}

void SkyboxPass::execute(const SceneRenderInfo         &scene_render_info,
                         const ViewRenderInfo          &view_render_info,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer)
{
  scene_framebuffer->bind();

  sky_box_shader_->bind();
  glActiveTexture(GL_TEXTURE0);
  const auto &sky = scene_render_info.env_map();
  if (is_show_irradiance_as_skybox_)
  {
    sky.env_irradiance_texture()->bind();
  }
  else
  {
    sky.env_texture()->bind();
  }
  sky_box_shader_->set_uniform("env_tex", 0);
  sky_box_shader_->set_uniform("projection_matrix",
                               view_render_info.projection_matrix());
  sky_box_shader_->set_uniform("view_matrix", view_render_info.view_matrix());
  glCullFace(GL_FRONT);
  render_cube();
  glCullFace(GL_BACK);

  scene_framebuffer->unbind();

  if (output_)
  {
    output_(scene_render_info, view_render_info, scene_framebuffer);
  }
}

void SkyboxPass::set_output(Output output) { output_ = output; }

void SkyboxPass::init_shaders()
{
  sky_box_shader_ = std::make_shared<GlShader>();
  sky_box_shader_->init("shaders/sky_box.vert", "shaders/sky_box.frag");
}

void SkyboxPass::init_cube()
{
  if (cube_vertex_array_ != 0)
  {
    return;
  }
  // clang-format off
  float vertices[]{
    // back face
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    // left face
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    // right face
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
    // bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    // top face
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
  };
  // clang-format on
  glGenVertexArrays(1, &cube_vertex_array_);
  glGenBuffers(1, &cube_vertex_buffer_);
  // fill buffer
  glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // link vertex attributes
  glBindVertexArray(cube_vertex_array_);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(float),
                        reinterpret_cast<void *>(6 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void SkyboxPass::render_cube()
{
  init_cube();

  glBindVertexArray(cube_vertex_array_);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

} // namespace dc
