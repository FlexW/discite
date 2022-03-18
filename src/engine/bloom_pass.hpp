#pragma once

#include "frame_data.hpp"
#include "gl_shader.hpp"
#include "gl_texture_view.hpp"

#include <array>
#include <functional>
#include <vector>

namespace dc
{

class RendererPanel;

class BloomPass
{
public:
  using Output =
      std::function<void(const SceneRenderInfo &        scene_render_info,
                         const ViewRenderInfo &         view_render_info,
                         std::shared_ptr<GlFramebuffer> scene_framebuffer,
                         std::shared_ptr<GlTexture>     bloom_texture)>;

  BloomPass();

  void execute(const SceneRenderInfo &        scene_render_info,
               const ViewRenderInfo &         view_render_info,
               std::shared_ptr<GlFramebuffer> scene_framebuffer);

  void set_output(const Output &output);

private:
  friend RendererPanel;

  static constexpr auto bloom_workgroup_size{4};

  Output output_;

  float threshold_{1.0f};
  float knee_{0.1f};
  float upsample_scale_{1.0f};
  float intensity_{1.0f};
  float dirt_intensity_{1.0f};

  std::array<std::shared_ptr<GlTexture>, 3>                  bloom_textures_{};
  std::array<std::vector<std::shared_ptr<GlTextureView>>, 3> bloom_views_;

  std::shared_ptr<GlShader> bloom_shader_{};

  void recreate_bloom_texture(const GlFramebuffer &scene_framebuffer);
};

} // namespace dc