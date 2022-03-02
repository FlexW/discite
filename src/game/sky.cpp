// #include "sky.hpp"

// // Sky::Sky(const std::filesystem::path &file_path)
// // {
// //   const std::filesystem::path irradiance_file_path{
// //       file_path.parent_path() /
// //       (file_path.stem().string() + "_irradiance.hdr")};

// //   env_texture_ = std::make_shared<GlCubeTexture>(file_path);
// //   env_irradiance_texture_ =
// //       std::make_shared<GlCubeTexture>(irradiance_file_path);
// // }

// std::shared_ptr<EnvionmentMap> Sky::environment_map() const
// {
//   return environment_map_;
// }

// // std::shared_ptr<GlCubeTexture> Sky::env_texture() const
// // {
// //   return env_texture_;
// // }

// // std::shared_ptr<GlCubeTexture> Sky::env_irradiance_texture() const
// // {
// //   return env_irradiance_texture_;
// // }

// void Sky::save(FILE *file) const {}

// void Sky::read(FILE *file) {}
