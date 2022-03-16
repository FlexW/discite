#pragma once

#include "engine.hpp"
#include "forward_pass.hpp"
#include "hdr_pass.hpp"
#include "render_object.hpp"
#include "shadow_pass.hpp"
#include "skybox_pass.hpp"

#include <memory>

namespace dc
{

class SceneRenderer
{
public:
  SceneRenderer();

  void render(const SceneRenderInfo &scene_render_info,
              const ViewRenderInfo & view_render_info);

  void register_object(std::shared_ptr<RenderObject> object);
  void unregister_object(std::shared_ptr<RenderObject> object);

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  using RenderObjectsMap =
      std::unordered_map<RenderObjectId, std::shared_ptr<RenderObject>>;

  RenderObjectsMap render_objects_;

  std::unique_ptr<ShadowPass>  shadow_pass_{std::make_unique<ShadowPass>()};
  std::unique_ptr<ForwardPass> forward_pass_{std::make_unique<ForwardPass>()};
  std::unique_ptr<SkyboxPass>  skybox_pass_{std::make_unique<SkyboxPass>()};
  std::unique_ptr<HdrPass>     hdr_pass_{std::make_unique<HdrPass>()};
};

} // namespace dc
