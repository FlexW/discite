#include "renderer.hpp"

namespace dc
{

Renderer::Renderer()
{
  shadow_pass_->set_output(
      [this](const SceneRenderInfo          &scene_render_info,
             const ViewRenderInfo           &view_render_info,
             std::shared_ptr<GlTextureArray> shadow_tex_array,
             const std::vector<glm::mat4>    light_space_matrices,
             const std::vector<CascadeSplit> cascade_frustums)
      {
        forward_pass_->execute(scene_render_info,
                               view_render_info,
                               shadow_tex_array,
                               light_space_matrices,
                               cascade_frustums);
      });

  forward_pass_->set_output(
      [this](const SceneRenderInfo         &scene_render_info,
             const ViewRenderInfo          &view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer)
      {
        skybox_pass_->execute(scene_render_info,
                              view_render_info,
                              scene_framebuffer);
      });

  skybox_pass_->set_output(
      [this](const SceneRenderInfo         &scene_render_info,
             const ViewRenderInfo          &view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer)
      {
        hdr_pass_->execute(scene_render_info,
                           view_render_info,
                           scene_framebuffer);
      });
}

void Renderer::render(const SceneRenderInfo &scene_render_info,
                      const ViewRenderInfo  &view_render_info)
{
  // executes all passes
  shadow_pass_->execute(scene_render_info, view_render_info);
}

} // namespace dc
