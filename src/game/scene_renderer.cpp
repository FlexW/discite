#include "scene_renderer.hpp"
#include "assert.hpp"
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
             std::shared_ptr<GlFramebuffer> scene_framebuffer)
      {
        skybox_pass_->execute(scene_render_info,
                              view_render_info,
                              scene_framebuffer);
      });

  skybox_pass_->set_output(
      [this](const SceneRenderInfo &        scene_render_info,
             const ViewRenderInfo &         view_render_info,
             std::shared_ptr<GlFramebuffer> scene_framebuffer)
      {
        hdr_pass_->execute(scene_render_info,
                           view_render_info,
                           scene_framebuffer);
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

void SceneRenderer::register_object(std::shared_ptr<RenderObject> object)
{
  const auto iter = render_objects_.find(object->id());
  if (iter != render_objects_.end())
  {
    DC_FAIL("Can not register scene renderer object twice");
    return;
  }

  render_objects_[object->id()] = object;

  forward_pass_->register_object(object);
}

void SceneRenderer::unregister_object(std::shared_ptr<RenderObject> object)
{
  const auto iter = render_objects_.find(object->id());
  if (iter == render_objects_.end())
  {
    DC_FAIL("Can not unregister scene renderer object");
    return;
  }

  forward_pass_->unregister_object(object);

  render_objects_.erase(iter);
}

} // namespace dc
