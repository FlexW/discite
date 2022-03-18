#include "renderer_panel.hpp"
#include "profiling.hpp"
#include "imgui.hpp"
#include "imgui_panel.hpp"

namespace dc
{

RendererPanel::RendererPanel() : ImGuiPanel{"Renderer"}
{
  glGenVertexArrays(1, &quad_vertex_array_);

  depth_debug_shader_ = std::make_shared<GlShader>();
  depth_debug_shader_->init("shaders/quad.vert", "shaders/debug_depth.frag");
}

RendererPanel::~RendererPanel()
{
  if (quad_vertex_array_)
  {
    glDeleteVertexArrays(1, &quad_vertex_array_);
  }
}

void RendererPanel::on_render()
{
  DC_PROFILE_SCOPE("RendererPanel::on_render()");

  if (ImGui::BeginTabBar("Renderer tabs"))
  {
    if (ImGui::BeginTabItem("General"))
    {
      render_general();
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Shadows"))
    {
      render_shadows();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

void RendererPanel::render_general()
{
  const auto renderer = renderer_.lock();
  if (!renderer)
  {
    return;
  }

  imgui_input("Irradiance sky box",
              renderer->skybox_pass_->is_show_irradiance_as_skybox_);

  imgui_input("Exposure", renderer->hdr_pass_->exposure_);
  imgui_input("Bloom intensity", renderer->hdr_pass_->bloom_intensity_);
  imgui_input("Bloom threshold", renderer->bloom_pass_->threshold_);
}

void RendererPanel::render_shadows()
{
  const auto renderer = renderer_.lock();
  if (!renderer)
  {
    return;
  }

  ImGui::Checkbox("Enable shadows",
                  &renderer->forward_pass_->is_shadows_enabled_);
  if (!renderer->forward_pass_->is_shadows_enabled_)
  {
    return;
  }
  ImGui::Checkbox("Show cascades",
                  &renderer->forward_pass_->show_shadow_cascades_);

  imgui_input("Light size", renderer->forward_pass_->light_size_);
  imgui_input("Bias", renderer->forward_pass_->shadow_bias_min_);
  imgui_input("Smooth", renderer->forward_pass_->smooth_shadows_);

  const int color_tex_width  = 1024;
  const int color_tex_height = 1024;

  recreate_debug_quad_framebuffer(color_tex_width, color_tex_height);

  // let the user select a cascade
  const auto selected_cascade = std::to_string(debug_selected_cascade_);
  if (ImGui::BeginCombo("Cascade", selected_cascade.c_str()))
  {
    for (std::size_t i = 0;
         i < renderer->shadow_pass_->cascade_frustums_.size();
         ++i)
    {
      const auto is_selected = i == debug_selected_cascade_;
      const auto cascade_str = std::to_string(i);
      if (ImGui::Selectable(cascade_str.c_str(), is_selected))
      {
        debug_selected_cascade_ = i;
      }

      if (is_selected)
      {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  // render a debug quad with the selected cascade
  {
    debug_quad_framebuffer_->bind();
    glViewport(0, 0, color_tex_width, color_tex_height);
    const glm::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
    glClearNamedFramebufferfv(debug_quad_framebuffer_->id(),
                              GL_COLOR,
                              0,
                              glm::value_ptr(clear_color));
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    depth_debug_shader_->bind();
    renderer->shadow_pass_->shadow_tex_array_->bind_unit(0);
    depth_debug_shader_->set_uniform("depth_tex", 0);
    depth_debug_shader_->set_uniform("layer",
                                     static_cast<int>(debug_selected_cascade_));
    glBindVertexArray(quad_vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    depth_debug_shader_->unbind();
    debug_quad_framebuffer_->unbind();
  }

  // display the cascade to the user
  const auto area = ImGui::GetContentRegionAvail();
  const auto tex  = std::get<std::shared_ptr<GlTexture>>(
      debug_quad_framebuffer_->color_attachment(0));
  ImGui::Text("Cascades");
  ImGui::Image(reinterpret_cast<void *>(tex->id()), {area.x, area.x});
}

void RendererPanel::set_renderer(std::shared_ptr<SceneRenderer> renderer)
{
  renderer_ = renderer;
}

void RendererPanel::recreate_debug_quad_framebuffer(int new_width,
                                                    int new_height)
{
  if (debug_quad_width_ == new_width && debug_quad_height_ == new_height)
  {
    return;
  }

  debug_quad_width_  = new_width;
  debug_quad_height_ = new_height;

  FramebufferAttachmentCreateConfig color_attachment_config{};
  color_attachment_config.format_          = GL_RGB;
  color_attachment_config.internal_format_ = GL_RGB32F;
  color_attachment_config.width_           = debug_quad_width_;
  color_attachment_config.height_          = debug_quad_height_;
  color_attachment_config.type_            = AttachmentType::Texture;

  FramebufferConfig framebuffer_config{};
  framebuffer_config.color_attachments_.push_back(color_attachment_config);

  debug_quad_framebuffer_ = std::make_shared<GlFramebuffer>();
  debug_quad_framebuffer_->attach(framebuffer_config);
}

} // namespace dc
