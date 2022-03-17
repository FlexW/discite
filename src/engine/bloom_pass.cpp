#include "bloom_pass.hpp"
#include "log.hpp"

namespace dc
{

BloomPass::BloomPass()
{
  bloom_shader_ = std::make_shared<GlShader>();
  bloom_shader_->init("shaders/bloom.comp");
}

void BloomPass::execute(const SceneRenderInfo &        scene_render_info,
                        const ViewRenderInfo &         view_render_info,
                        std::shared_ptr<GlFramebuffer> scene_framebuffer)
{
  recreate_bloom_texture(*scene_framebuffer);

  const auto scene_texture = std::get<std::shared_ptr<GlTexture>>(
      scene_framebuffer->color_attachment(0));

  float lod{0.0f};
  float mode{0}; // 0 = prefilter, 1 = downsample, 2 = firstsample, 3 = upsample

  bloom_shader_->bind();
  bloom_shader_->set_uniform(
      "Params",
      glm::vec4{threshold_, threshold_ - knee_, knee_ * 2.0f, 0.25f / knee_});
  bloom_shader_->set_uniform("Mode", 0);
  bloom_shader_->set_uniform("LOD", 0.0f);

  // bloom_views_[0][0]->bind_unit(0);
  glBindImageTexture(0,
                     bloom_views_[0][0]->id(),
                     0,
                     GL_FALSE,
                     0,
                     GL_READ_ONLY,
                     GL_RGBA32F);
  // bloom_shader_->set_uniform("o_Image", 0);

  scene_texture->bind_unit(1);
  bloom_shader_->set_uniform("u_Texture", 1);

  scene_texture->bind_unit(2);
  bloom_shader_->set_uniform("u_BloomTexture", 2);

  const auto work_groups_x = bloom_textures_[0]->width() / bloom_workgroup_size;
  const auto work_groups_y =
      bloom_textures_[0]->height() / bloom_workgroup_size;
  compute(work_groups_x, work_groups_y, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  if (output_)
  {
    output_(scene_render_info, view_render_info, scene_framebuffer);
  }
}

void BloomPass::recreate_bloom_texture(const GlFramebuffer &scene_framebuffer)
{
  const auto scene_texture = std::get<std::shared_ptr<GlTexture>>(
      scene_framebuffer.color_attachment(0));

  auto width  = scene_texture->width();
  auto height = scene_texture->height();
  width += (bloom_workgroup_size - (width % bloom_workgroup_size));
  height += (bloom_workgroup_size - (height % bloom_workgroup_size));

  if (bloom_textures_[0] && bloom_textures_[0]->width() == width &&
      bloom_textures_[0]->height() == height)
  {
    return;
  }

  GlTextureConfig texture_config{};
  texture_config.width_            = width;
  texture_config.height_           = height;
  texture_config.format_           = GL_RGBA;
  texture_config.sized_format_     = GL_RGBA32F;
  texture_config.wrap_s_           = GL_CLAMP_TO_EDGE;
  texture_config.wrap_t_           = GL_CLAMP_TO_EDGE;
  texture_config.min_filter_       = GL_LINEAR_MIPMAP_LINEAR;
  texture_config.mag_filter_       = GL_LINEAR;
  texture_config.generate_mipmaps_ = true;

  for (std::size_t i = 0; i < bloom_textures_.size(); ++i)
  {
    bloom_textures_[i] = std::make_shared<GlTexture>(texture_config);

    // generate texture views for the mipmaps
    for (GLuint j = 0; j < bloom_textures_[i]->mipmap_levels(); ++j)
    {
      GlTextureViewConfig view_config{};
      view_config.texture_    = bloom_textures_[i];
      view_config.min_level_  = j;
      view_config.num_levels_ = 1;
      view_config.min_layer_ = 0;
      view_config.num_layers_ = 1;

      const auto view = std::make_shared<GlTextureView>(view_config);
      bloom_views_[i].push_back(view);
    }
  }
}

void BloomPass::set_output(const Output &output) { output_ = output; }

} // namespace dc