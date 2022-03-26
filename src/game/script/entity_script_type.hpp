#pragma once

#include "uuid.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

#include <string>

namespace dc
{

struct EntityScriptType
{
  std::string full_name_;
  std::string namespace_name_;
  std::string class_name_;

  MonoClass *mono_class_{};

  MonoMethod *constructor_{};
  MonoMethod *method_on_update_{};

  EntityScriptType(const std::string &full_name,
                   const std::string &namespace_name,
                   const std::string &class_name,
                   MonoImage         *core_image,
                   MonoImage         *game_image);

  void construct(MonoObject *object, Uuid id);
  void on_update(MonoObject *object, float delta_time);

private:
  static MonoClass  *get_class(MonoImage         *image,
                               const std::string &namespace_name,
                               const std::string &class_name);
  static MonoMethod *get_method(MonoImage *image, const std::string &name);
  static MonoObject *
  call_method(MonoObject *object, MonoMethod *method, void **params = nullptr);
  static std::string get_string_property(const std::string &property_name,
                                         MonoClass         *class_type,
                                         MonoObject        *object);

  void init_methods(MonoImage *core_image, MonoImage *image);
};

} // namespace dc
