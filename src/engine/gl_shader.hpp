#pragma once

#include "gl_index_buffer.hpp"
#include "gl_vertex_buffer.hpp"
#include "math.hpp"

#include <glad/glad.h>

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

class GlShader
{
public:
  GlShader() = default;
  ~GlShader();

  void init(const std::filesystem::path &vertex_shader_file_path,
            const std::filesystem::path &fragment_shader_file_path);

  void bind();
  void unbind();

  void set_uniform(const std::string &name, bool value);
  void set_uniform(const std::string &name, int value);
  void set_uniform(const std::string &name, float value);
  void set_uniform(const std::string &name, const glm::vec3 &value);
  void set_uniform(const std::string &name, const glm::vec4 &value);
  void set_uniform(const std::string &name, const glm::mat2 &value);
  void set_uniform(const std::string &name, const glm::mat4 &value);

private:
  GLuint program_id_{};

  std::unordered_map<std::string, GLint> uniform_locations_;

  GlShader(const GlShader &) = delete;
  void operator=(const GlShader &) = delete;
  GlShader(GlShader &&)            = delete;
  void operator=(GlShader &&) = delete;

  [[nodiscard]] GLint uniform_location(const std::string &name);
};
