#pragma once

#include "gl_index_buffer.hpp"
#include "gl_vertex_buffer.hpp"
#include "math.hpp"

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace dc
{

class GlShader
{
public:
  GlShader() = default;
  GlShader(const std::filesystem::path &vertex_shader_file_path,
           const std::filesystem::path &fragment_shader_file_path);

  GlShader(const std::filesystem::path &vertex_shader_file_path,
           const std::filesystem::path &geometry_shader_file_path,
           const std::filesystem::path &fragment_shader_file_path);

  GlShader(const std::filesystem::path &compute_shader_file_path);
  ~GlShader();

  void init(const std::filesystem::path &vertex_shader_file_path,
            const std::filesystem::path &fragment_shader_file_path);

  void init(const std::filesystem::path &vertex_shader_file_path,
            const std::filesystem::path &geometry_shader_file_path,
            const std::filesystem::path &fragment_shader_file_path);

  void init(const std::filesystem::path &compute_shader_file_path);

  void bind();
  void unbind();

  void set_uniform(const std::string &name, bool value);
  void set_uniform(const std::string &name, int value);
  void set_uniform(const std::string &name, float value);
  void set_uniform(const std::string &name, const glm::vec3 &value);
  void set_uniform(const std::string &name, const glm::vec4 &value);
  void set_uniform(const std::string &name, const glm::mat2 &value);
  void set_uniform(const std::string &name, const glm::mat4 &value);
  void set_uniform(const std::string            &name,
                   const std::vector<glm::mat4> &value);
  void set_uniform(const std::string &name, const std::vector<float> &value);

private:
  struct UniformInfo
  {
    GLint   location{};
    GLsizei count{};
  };

  GLuint program_id_{};

  std::unordered_map<std::string, UniformInfo> uniforms_;

  GlShader(const GlShader &) = delete;
  void operator=(const GlShader &) = delete;
  GlShader(GlShader &&)            = delete;
  void operator=(GlShader &&) = delete;

  [[nodiscard]] GLint uniform_location(const std::string &name);

  GLuint compile_shader(const std::string &shader_code, GLenum type);
  GLuint compile_shader(const std::filesystem::path &file_path);
  void   link_shaders(const std::vector<GLuint> &shader_ids);

  void load_shader_data();
};

} // namespace dc
