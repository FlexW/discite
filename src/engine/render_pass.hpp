// #pragma once

// #include "frame_data.hpp"
// #include "gl_cube_texture.hpp"
// #include "gl_framebuffer.hpp"
// #include "gl_renderbuffer.hpp"
// #include "gl_texture.hpp"
// #include "gl_texture_array.hpp"

// #include <cstdint>
// #include <memory>
// #include <string>
// #include <unordered_map>
// #include <variant>

// namespace dc
// {

// struct CascadeSplit
// {
//   float near{};
//   float far{};
// };

// using RenderVarying = std::variant<std::shared_ptr<GlTexture>,
//                                    std::shared_ptr<GlTextureArray>,
//                                    std::shared_ptr<GlRenderbuffer>,
//                                    std::shared_ptr<GlFramebuffer>,
//                                    std::shared_ptr<GlCubeTexture>,
//                                    std::uint32_t,
//                                    std::int32_t,
//                                    float,
//                                    std::vector<glm::mat4>,
//                                    glm::vec3,
//                                    glm::vec4,
//                                    bool,
//                                    std::vector<CascadeSplit>>;

// class RenderVaryings
// {
// public:
//   void set_render_varying(const std::string &name, RenderVarying varying);

//   RenderVarying render_varying(const std::string &name) const;
//   std::shared_ptr<GlTextureArray> texture_array(const std::string &name)
//   const; std::shared_ptr<GlTexture>      texture(const std::string &name)
//   const; std::shared_ptr<GlCubeTexture>  cube_texture(const std::string
//   &name) const; std::shared_ptr<GlCubeTexture>  framebuffer(const std::string
//   &name) const; std::shared_ptr<GlCubeTexture>  renderbuffer(const
//   std::string &name) const; std::vector<glm::mat4>          matrices(const
//   std::string &name) const;

// private:
//   std::unordered_map<std::string, RenderVarying> names_to_varyings_;
// };

// class RenderPass
// {
// public:
//   virtual ~RenderPass() = default;

//   void connect(std::shared_ptr<RenderPass> render_pass);

//   virtual void receive(RenderVaryings        &render_varyings,
//                        const SceneRenderInfo &scene_render_info,
//                        const ViewRenderInfo  &view_render_info) = 0;

// protected:
//   void deliver(RenderVaryings        &render_varyings,
//                const SceneRenderInfo &scene_render_info,
//                const ViewRenderInfo  &view_render_info);

// private:
//   std::shared_ptr<RenderPass> connected_pass_{};
// };

// } // namespace dc
