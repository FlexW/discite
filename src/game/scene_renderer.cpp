#include "scene_renderer.hpp"
#include "profiling.hpp"
#include "time.hpp"

namespace dc
{

SceneRenderer::SceneRenderer()
{
  shadow_pass_->set_output(
      [this](const SceneRenderInfo &         scene_render_info,
             const ViewRenderInfo &          view_render_info,
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
      [this](const SceneRenderInfo &        scene_render_info,
             const ViewRenderInfo &         view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer,
             std::shared_ptr<GlCubeTexture> sky_irradiance_map)
      {
        skybox_pass_->execute(scene_render_info,
                              view_render_info,
                              scene_framebuffer,
                              sky_irradiance_map);
      });

  bloom_pass_->set_output(
      [this](const SceneRenderInfo &        scene_render_info,
             const ViewRenderInfo &         view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer,
             std::shared_ptr<GlTexture>     bloom_texture)
      {
        hdr_pass_->execute(scene_render_info,
                           view_render_info,
                           scene_framebuffer,
                           bloom_texture);
      });

  skybox_pass_->set_output(
      [this](const SceneRenderInfo &        scene_render_info,
             const ViewRenderInfo &         view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer)
      {
        if (is_bloom_enabled_)
        {
          bloom_pass_->execute(scene_render_info,
                               view_render_info,
                               scene_framebuffer);
        }
        else
        {
          hdr_pass_->execute(scene_render_info,
                             view_render_info,
                             scene_framebuffer,
                             nullptr);
        }
      });
}

void SceneRenderer::render(const SceneRenderInfo &scene_render_info,
                           const ViewRenderInfo & view_render_info)
{
  DC_PROFILE_SCOPE("SceneRenderer::render()");
  DC_TIME_SCOPE_PERF("Scene renderer render");
  // executes all passes
  shadow_pass_->execute(scene_render_info, view_render_info);
}

} // namespace dc
