// #include "render_pass.hpp"
// #include "log.hpp"

// namespace dc
// {

// void RenderVaryings::set_render_varying(const std::string &name,
//                                         RenderVarying      varying)
// {
//   names_to_varyings_[name] = varying;
// }

// RenderVarying RenderVaryings::render_varying(const std::string &name)
// {
//   const auto iter = names_to_varyings_.find(name);
//   if (iter != names_to_varyings_.end())
//   {
//     return iter->second;
//   }

//   DC_LOG_WARN("Can not find render varying: ", name);

//   return {};
// }

// void RenderPass::connect(std::shared_ptr<RenderPass> render_pass)
// {
//   connected_pass_ = render_pass;
// }

// void RenderPass::deliver(RenderVaryings        &render_varyings,
//                          const SceneRenderInfo &scene_render_info,
//                          const ViewRenderInfo  &view_render_info)
// {
//   if (connected_pass_)
//   {
//     connected_pass_->receive(render_varyings,
//                              scene_render_info,
//                              view_render_info);
//   }
// }

// } // namespace dc
