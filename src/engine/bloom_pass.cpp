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
  int   mode{0}; // 0 = prefilter, 1 = downsample, 2 = firstsample, 3 = upsample

  // prefilter
  bloom_shader_->bind();
  bloom_shader_->set_uniform(
      "Params",
      glm::vec4{threshold_, threshold_ - knee_, knee_ * 2.0f, 0.25f / knee_});
  mode = 0;
  bloom_shader_->set_uniform("Mode", mode);
  bloom_shader_->set_uniform("LOD", 0.0f);

  glBindImageTexture(0,
                     bloom_views_[0][0]->id(),
                     0,
                     GL_FALSE,
                     0,
                     GL_READ_ONLY,
                     GL_RGBA32F);

  scene_texture->bind_unit(1);
  bloom_shader_->set_uniform("u_Texture", 1); // Not needed

  scene_texture->bind_unit(2);
  bloom_shader_->set_uniform("u_BloomTexture", 2); // Not needed

  auto work_groups_x = bloom_textures_[0]->width() / bloom_workgroup_size;
  auto work_groups_y = bloom_textures_[0]->height() / bloom_workgroup_size;
  compute(work_groups_x, work_groups_y, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // downsample
  mode = 1;
  bloom_shader_->set_uniform("Mode", mode);
  const auto mips = bloom_textures_[0]->mipmap_levels() - 2;
  for (GLuint i = 1; i < mips; ++i)
  {
    glBindImageTexture(0,
                       bloom_views_[0][i]->id(),
                       0,
                       GL_FALSE,
                       0,
                       GL_READ_ONLY,
                       GL_RGBA32F);

    bloom_textures_[0]->bind_unit(1);

    lod = i - 1.0f;
    bloom_shader_->set_uniform("LOD", lod);

    const auto [mip_width, mip_height] = bloom_textures_[0]->mipmap_size(i);
    work_groups_x = glm::ceil(static_cast<float>(mip_width) /
                              static_cast<float>(bloom_workgroup_size));
    work_groups_y = glm::ceil(static_cast<float>(mip_height) /
                              static_cast<float>(bloom_workgroup_size));

    compute(work_groups_x, work_groups_y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  // upsample first
  --lod;
  bloom_shader_->set_uniform("LOD", lod);
  mode = 2;
  bloom_shader_->set_uniform("Mode", mode);

  work_groups_x *= 2;
  work_groups_y *= 2;

  bloom_textures_[0]->bind_unit(1);

  glBindImageTexture(0,
                     bloom_views_[1][mips - 2]->id(),
                     0,
                     GL_FALSE,
                     0,
                     GL_READ_ONLY,
                     GL_RGBA32F);

  const auto [mip_width, mip_height] =
      bloom_textures_[1]->mipmap_size(mips - 2);
  work_groups_x = glm::ceil(static_cast<float>(mip_width) /
                            static_cast<float>(bloom_workgroup_size));
  work_groups_y = glm::ceil(static_cast<float>(mip_height) /
                            static_cast<float>(bloom_workgroup_size));

  compute(work_groups_x, work_groups_y, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // upsample
  mode = 3;
  bloom_shader_->set_uniform("Mode", mode);

  for (int mip = mips - 3; mip >= 0; --mip)
  {
    const auto [mip_width, mip_height] = bloom_textures_[1]->mipmap_size(mip);
    work_groups_x = glm::ceil(static_cast<float>(mip_width) /
                              static_cast<float>(bloom_workgroup_size));
    work_groups_y = glm::ceil(static_cast<float>(mip_height) /
                              static_cast<float>(bloom_workgroup_size));

    glBindImageTexture(0,
                       bloom_views_[1][mip]->id(),
                       0,
                       GL_FALSE,
                       0,
                       GL_READ_ONLY,
                       GL_RGBA32F);

    bloom_textures_[1]->bind_unit(2);
    lod = mip;
    bloom_shader_->set_uniform("LOD", lod);

    compute(work_groups_x, work_groups_y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  bloom_shader_->unbind();

  if (output_)
  {
    output_(scene_render_info,
            view_render_info,
            scene_framebuffer,
            bloom_textures_[1]);
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
    bloom_views_[i].clear();

    // generate texture views for the mipmaps
    for (GLuint j = 0; j < bloom_textures_[i]->mipmap_levels(); ++j)
    {
      GlTextureViewConfig view_config{};
      view_config.texture_    = bloom_textures_[i];
      view_config.min_level_  = j;
      view_config.num_levels_ = 1;
      view_config.min_layer_  = 0;
      view_config.num_layers_ = 1;

      const auto view = std::make_shared<GlTextureView>(view_config);
      bloom_views_[i].push_back(view);
    }
  }
}

void BloomPass::set_output(const Output &output) { output_ = output; }

} // namespace dc
