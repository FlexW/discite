#include "renderer_panel.hpp"
#include "imgui.hpp"
#include "imgui_panel.hpp"

RendererPanel::RendererPanel() : ImGuiPanel{"Renderer"} {}

void RendererPanel::on_render()
{
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
}

void RendererPanel::render_shadows()
{
  const auto renderer = renderer_.lock();
  if (!renderer)
  {
    return;
  }

  ImGui::Checkbox("Enable shadows", &renderer->is_shadows_enabled_);
  if (!renderer->is_shadows_enabled_)
  {
    return;
  }
  ImGui::Checkbox("Show cascades", &renderer->show_shadow_cascades_);

  imgui_input("Light size", renderer->light_size_);
  imgui_input("Bias", renderer->shadow_bias_min_);
  imgui_input("Smooth", renderer->smooth_shadows_);

  const int color_tex_width  = 1024;
  const int color_tex_height = 1024;
  renderer->recreate_debug_quad_framebuffer(color_tex_width, color_tex_height);

  // let the user select a cascade
  const auto selected_cascade =
      std::to_string(renderer->debug_selected_cascade_);
  if (ImGui::BeginCombo("Cascade", selected_cascade.c_str()))
  {
    for (std::size_t i = 0; i < renderer->cascade_frustums_.size(); ++i)
    {
      const auto is_selected = i == renderer->debug_selected_cascade_;
      const auto cascade_str = std::to_string(i);
      if (ImGui::Selectable(cascade_str.c_str(), is_selected))
      {
        renderer->debug_selected_cascade_ = i;
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
    renderer->debug_quad_framebuffer_->bind();
    glViewport(0, 0, color_tex_width, color_tex_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    renderer->depth_debug_shader_->bind();
    glActiveTexture(GL_TEXTURE0);
    renderer->shadow_tex_array_->bind();
    renderer->depth_debug_shader_->set_uniform("depth_tex", 0);
    renderer->depth_debug_shader_->set_uniform(
        "layer",
        static_cast<int>(renderer->debug_selected_cascade_));
    glBindVertexArray(renderer->quad_vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    renderer->depth_debug_shader_->unbind();
    renderer->debug_quad_framebuffer_->unbind();
  }

  // display the cascade to the user
  const auto area = ImGui::GetContentRegionAvail();
  const auto tex  = std::get<std::shared_ptr<GlTexture>>(
      renderer->debug_quad_framebuffer_->color_attachment(0));
  ImGui::Text("Cascades");
  ImGui::Image(reinterpret_cast<void *>(tex->id()), {area.x, area.x});
}

void RendererPanel::set_renderer(std::shared_ptr<Renderer> renderer)
{
  renderer_ = renderer;
}
