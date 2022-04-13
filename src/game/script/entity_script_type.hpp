#pragma once

#include "entity.hpp"
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
  MonoMethod *method_on_create_{};
  MonoMethod *method_on_update_{};
  MonoMethod *method_on_collision_begin_{};
  MonoMethod *method_on_collision_end_{};
  MonoMethod *method_on_trigger_begin_{};
  MonoMethod *method_on_trigger_end_{};

  EntityScriptType(const std::string &full_name,
                   const std::string &namespace_name,
                   const std::string &class_name,
                   MonoImage         *core_image,
                   MonoImage         *game_image);

  void construct(MonoObject *object, Uuid id);
  void on_create(MonoObject *object);
  void on_update(MonoObject *object, float delta_time);
  void on_collision_begin(MonoObject *object, Entity collidee);
  void on_collision_end(MonoObject *object, Entity collidee);
  void on_trigger_begin(MonoObject *object, Entity other);
  void on_trigger_end(MonoObject *object, Entity other);

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
