#pragma once

#include "bloom_pass.hpp"
#include "engine.hpp"
#include "forward_pass.hpp"
#include "hdr_pass.hpp"
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

private:
  // TODO: Workaround. Expose public API
  friend class RendererPanel;

  bool is_bloom_enabled_{true};

  std::unique_ptr<ShadowPass>  shadow_pass_{std::make_unique<ShadowPass>()};
  std::unique_ptr<ForwardPass> forward_pass_{std::make_unique<ForwardPass>()};
  std::unique_ptr<BloomPass>   bloom_pass_{std::make_unique<BloomPass>()};
  std::unique_ptr<SkyboxPass>  skybox_pass_{std::make_unique<SkyboxPass>()};
  std::unique_ptr<HdrPass>     hdr_pass_{std::make_unique<HdrPass>()};
};

} // namespace dc
