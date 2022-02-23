#include "gl_shader.hpp"
#include "defer.hpp"
#include "log.hpp"

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
std::string read_text_file(const std::filesystem::path &file_path)
{
  std::ifstream ifs(file_path);
  if (!ifs.is_open())
  {
    throw std::runtime_error(std::string("Could not open file ") +
                             file_path.string());
  }
  std::string str(
      static_cast<std::stringstream const &>(std::stringstream() << ifs.rdbuf())
          .str());

  return str;
}

void check_for_shader_compile_errors(const std::string &file_name,
                                     GLuint             shader_id)
{
  GLint     success       = false;
  const int info_log_size = 1024;
  GLchar    info_log[info_log_size];

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader_id, info_log_size, nullptr, info_log);

    const std::string message =
        " Shader compile error in " + file_name + ":\n" + info_log;
    throw std::runtime_error(message);
  }
}

void check_for_program_link_errors(GLuint program_id)
{
  GLint     success       = false;
  const int info_log_size = 1024;
  GLchar    info_log[info_log_size];
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(program_id, info_log_size, nullptr, info_log);

    const std::string message = std::string("Shader link error:\n ") + info_log;
    throw std::runtime_error(message);
  }
}

} // namespace

GlShader::GlShader(const std::filesystem::path &vertex_shader_file_path,
                   const std::filesystem::path &fragment_shader_file_path)
{
  init(vertex_shader_file_path, fragment_shader_file_path);
}

GlShader::GlShader(const std::filesystem::path &vertex_shader_file_path,
                   const std::filesystem::path &geometry_shader_file_path,
                   const std::filesystem::path &fragment_shader_file_path)
{
  init(vertex_shader_file_path,
       geometry_shader_file_path,
       fragment_shader_file_path);
}

GlShader::GlShader(const std::filesystem::path &compute_shader_file_path)
{
  init(compute_shader_file_path);
}

GlShader::~GlShader()
{
  if (program_id_)
  {
    glDeleteProgram(program_id_);
  }
}

GLuint GlShader::compile_shader(const std::filesystem::path &file_path)
{
  const auto extension = file_path.extension();
  GLuint     shader_id{};
  const auto shader_code = read_text_file(file_path);

  if (extension == ".vert")
  {
    shader_id = compile_shader(shader_code, GL_VERTEX_SHADER);
  }
  else if (extension == ".geom")
  {
    shader_id = compile_shader(shader_code, GL_GEOMETRY_SHADER);
  }
  else if (extension == ".frag")
  {
    shader_id = compile_shader(shader_code, GL_FRAGMENT_SHADER);
  }
  else if (extension == ".comp")
  {
    shader_id = compile_shader(shader_code, GL_COMPUTE_SHADER);
  }
  else
  {
    assert(0 && "Can not compile shader type");
  }
  check_for_shader_compile_errors(file_path.string(), shader_id);

  return shader_id;
}

GLuint GlShader::compile_shader(const std::string &shader_code, GLenum type)
{
  const auto shader_id         = glCreateShader(type);
  auto       shader_code_c_str = shader_code.c_str();
  glShaderSource(shader_id, 1, &shader_code_c_str, nullptr);
  glCompileShader(shader_id);

  return shader_id;
}

void GlShader::link_shaders(const std::vector<GLuint> &shader_ids)
{
  program_id_ = glCreateProgram();

  for (const auto &shader_id : shader_ids)
  {
    glAttachShader(program_id_, shader_id);
  }

  glLinkProgram(program_id_);
  check_for_program_link_errors(program_id_);
}

void GlShader::dump_shader_info()
{

  // Query informations
  GLint attribs_count = 0;
  glGetProgramiv(program_id_, GL_ACTIVE_ATTRIBUTES, &attribs_count);
  GLint max_attrib_name_length = 0;
  glGetProgramiv(program_id_,
                 GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
                 &max_attrib_name_length);

  LOG_DEBUG() << "Active attributes: " << attribs_count;
  std::vector<GLchar> attrib_name_data(max_attrib_name_length);
  for (GLint i = 0; i < attribs_count; ++i)
  {
    GLint   array_size    = 0;
    GLenum  type          = 0;
    GLsizei actual_length = 0;
    glGetActiveAttrib(program_id_,
                      i,
                      attrib_name_data.size(),
                      &actual_length,
                      &array_size,
                      &type,
                      attrib_name_data.data());
    std::string attrib_name(attrib_name_data.data(), actual_length);
    LOG_DEBUG() << "Attribute " << i << " " << attrib_name << " type: " << type
                << " size: " << array_size;
  }

  GLint uniforms_count = 0;
  glGetProgramInterfaceiv(program_id_,
                          GL_UNIFORM,
                          GL_ACTIVE_RESOURCES,
                          &uniforms_count);
  LOG_DEBUG() << "Active uniforms: " << uniforms_count;
  for (GLint i = 0; i < uniforms_count; ++i)
  {
    std::array<GLenum, 3> properties = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};
    std::array<GLint, 3>  results{};
    glGetProgramResourceiv(program_id_,
                           GL_UNIFORM,
                           i,
                           properties.size(),
                           properties.data(),
                           results.size(),
                           nullptr,
                           results.data());
    std::string name;
    const auto  name_length = results[0];
    name.resize(name_length);
    glGetProgramResourceName(program_id_,
                             GL_UNIFORM,
                             i,
                             name_length,
                             nullptr,
                             name.data());
    name[name.size() - 1] = '\0';
    const auto location   = results[2];
    LOG_DEBUG() << "Uniform " << location << " " << name;
    uniform_locations_[name] = location;
  }
}

void GlShader::init(const std::filesystem::path &vertex_shader_file_path,
                    const std::filesystem::path &fragment_shader_file_path)
{
  LOG_INFO() << "Load vertex shader " << vertex_shader_file_path.string();
  LOG_INFO() << "Load fragment shader " << fragment_shader_file_path.string();

  // compile shaders
  const auto vertex_shader_id = compile_shader(vertex_shader_file_path);
  defer(glDeleteShader(vertex_shader_id));

  const auto fragment_shader_id = compile_shader(fragment_shader_file_path);
  defer(glDeleteShader(fragment_shader_id));

  // link the shaders together
  link_shaders(std::vector<GLuint>{vertex_shader_id, fragment_shader_id});

  dump_shader_info();
}

void GlShader::init(const std::filesystem::path &vertex_shader_file_path,
                    const std::filesystem::path &geometry_shader_file_path,
                    const std::filesystem::path &fragment_shader_file_path)
{
  LOG_INFO() << "Load vertex shader " << vertex_shader_file_path.string();
  LOG_INFO() << "Load geometry shader " << geometry_shader_file_path.string();
  LOG_INFO() << "Load fragment shader " << fragment_shader_file_path.string();

  // compile shaders
  const auto vertex_shader_id = compile_shader(vertex_shader_file_path);
  defer(glDeleteShader(vertex_shader_id));

  const auto geometry_shader_id = compile_shader(geometry_shader_file_path);
  defer(glDeleteShader(geometry_shader_id));

  const auto fragment_shader_id = compile_shader(fragment_shader_file_path);
  defer(glDeleteShader(fragment_shader_id));

  // link the shaders together
  link_shaders(std::vector<GLuint>{vertex_shader_id,
                                   geometry_shader_id,
                                   fragment_shader_id});

  dump_shader_info();
}

void GlShader::init(const std::filesystem::path &compute_shader_file_path)
{
  LOG_INFO() << "Load compute shader " << compute_shader_file_path.string();

  // compile shaders
  const auto compute_shader_id = compile_shader(compute_shader_file_path);
  defer(glDeleteShader(compute_shader_id));

  link_shaders(std::vector<GLuint>{compute_shader_id});
}

void GlShader::bind() { glUseProgram(program_id_); }

void GlShader::unbind() { glUseProgram(0); }

GLint GlShader::uniform_location(const std::string &name)
{
  const auto iter = uniform_locations_.find(name);
  if (iter == uniform_locations_.end())
  {
    const auto location = glGetUniformLocation(program_id_, name.c_str());
    if (location == -1)
    {
      LOG_WARN() << "Could not find uniform " << name;
    }
    uniform_locations_[name] = location;
    return location;
  }
  return iter->second;
}

#define GET_UNIFORM_OR_RETURN(name, location)                                  \
  const auto location = uniform_location(name);                                \
  if (location == -1)                                                          \
  {                                                                            \
    return;                                                                    \
  }

void GlShader::set_uniform(const std::string &name, bool value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniform1i(location, static_cast<int>(value));
}

void GlShader::set_uniform(const std::string &name, int value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniform1i(location, static_cast<int>(value));
}

void GlShader::set_uniform(const std::string &name, float value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniform1f(location, value);
}

void GlShader::set_uniform(const std::string &name, const glm::vec3 &value)
{

  GET_UNIFORM_OR_RETURN(name, location)
  glUniform3fv(location, 1, glm::value_ptr(value));
}

void GlShader::set_uniform(const std::string &name, const glm::vec4 &value)
{

  GET_UNIFORM_OR_RETURN(name, location)
  glUniform4fv(location, 1, glm::value_ptr(value));
}

void GlShader::set_uniform(const std::string &name, const glm::mat2 &value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void GlShader::set_uniform(const std::string &name, const glm::mat4 &value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void GlShader::set_uniform(const std::string            &name,
                           const std::vector<glm::mat4> &value)
{
  GET_UNIFORM_OR_RETURN(name, location)
  glUniformMatrix4fv(location,
                     value.size(),
                     GL_FALSE,
                     glm::value_ptr(value[0]));
}
