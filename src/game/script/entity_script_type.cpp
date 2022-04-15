#include "entity_script_type.hpp"
#include "log.hpp"
#include "script_engine.hpp"
#include "uuid.hpp"

#include <mono/metadata/debug-helpers.h>

#include <array>

namespace dc
{

MonoMethod *EntityScriptType::get_method(MonoImage         *image,
                                         const std::string &name)
{
  const auto desc = mono_method_desc_new(name.c_str(), 0);
  if (!desc)
  {
    DC_LOG_DEBUG("Can not find method desc {}", name);
    return {};
  }

  const auto method = mono_method_desc_search_in_image(desc, image);
  if (!method)
  {
    DC_LOG_DEBUG("Can not find method {}", name);
    return {};
  }

  return method;
}

EntityScriptType::EntityScriptType(const std::string &full_name,
                                   const std::string &namespace_name,
                                   const std::string &class_name,
                                   MonoImage         *core_image,
                                   MonoImage         *game_image)
    : full_name_{full_name},
      namespace_name_{namespace_name},
      class_name_{class_name}
{
  mono_class_ = get_class(game_image, namespace_name_, class_name_);
  init_methods(core_image, game_image);
}

void EntityScriptType::init_methods(MonoImage *core_image,
                                    MonoImage *game_image)
{
  // base methods
  constructor_ = get_method(core_image, "Dc.Entity:.ctor(ulong)");
  DC_ASSERT(constructor_, "Method not set");

  method_on_collision_begin_ =
      get_method(core_image, "Dc.Entity:OnCollisionBeginInternal(ulong)");
  DC_ASSERT(method_on_collision_begin_, "Method not set");

  method_on_collision_end_ =
      get_method(core_image, "Dc.Entity:OnCollisionEndInternal(ulong)");
  DC_ASSERT(method_on_collision_end_, "Method not set");

  method_on_trigger_begin_ =
      get_method(core_image, "Dc.Entity:OnTriggerBeginInternal(ulong)");
  DC_ASSERT(method_on_trigger_begin_, "Method not set");

  method_on_trigger_end_ =
      get_method(core_image, "Dc.Entity:OnTriggerEndInternal(ulong)");
  DC_ASSERT(method_on_trigger_end_, "Method not set");

  // child methods
  method_on_create_ = get_method(game_image, full_name_ + ":OnCreate()");
  method_on_update_ = get_method(game_image, full_name_ + ":OnUpdate(single)");
}

MonoObject *EntityScriptType::call_method(MonoObject *object,
                                          MonoMethod *method,
                                          void      **params)
{
  DC_ASSERT(object, "No object set");
  DC_ASSERT(method, "No method set");

  MonoObject *exception{};
  const auto  result = mono_runtime_invoke(method, object, params, &exception);
  if (exception)
  {
    const auto exception_class = mono_object_get_class(exception);
    const auto exception_type  = mono_class_get_type(exception_class);
    const auto type_name       = mono_type_get_name(exception_type);
    const auto message =
        get_string_property("Message", exception_class, exception);
    const auto stack_trace =
        get_string_property("StackTrace", exception_class, exception);

    DC_LOG_WARN("{}: {}. StackTrace: {}", type_name, message, stack_trace);
  }
  return result;
}

void EntityScriptType::construct(MonoObject *object, Uuid id)
{
  std::array<void *, 1> param{{&id}};
  call_method(object, constructor_, param.data());
}

void EntityScriptType::on_create(MonoObject *object)
{
  if (method_on_create_)
  {
    call_method(object, method_on_create_);
  }
}

void EntityScriptType::on_update(MonoObject *object, float delta_time)
{
  if (method_on_update_)
  {
    std::array<void *, 1> args{{&delta_time}};
    call_method(object, method_on_update_, args.data());
  }
}

void EntityScriptType::on_collision_begin(MonoObject *object, Entity collidee)
{
  auto id = collidee.id();

  std::array<void *, 1> args{{&id}};
  call_method(object, method_on_collision_begin_, args.data());
}

void EntityScriptType::on_collision_end(MonoObject *object, Entity collidee)
{
  auto id = collidee.id();

  std::array<void *, 1> args{{&id}};
  call_method(object, method_on_collision_end_, args.data());
}

void EntityScriptType::on_trigger_begin(MonoObject *object, Entity other)
{
  auto id = other.id();

  std::array<void *, 1> args{{&id}};
  call_method(object, method_on_trigger_begin_, args.data());
}

void EntityScriptType::on_trigger_end(MonoObject *object, Entity other)
{
  auto id = other.id();

  std::array<void *, 1> args{{&id}};
  call_method(object, method_on_trigger_end_, args.data());
}

std::string
EntityScriptType::get_string_property(const std::string &property_name,
                                      MonoClass         *class_type,
                                      MonoObject        *object)
{
  const auto property =
      mono_class_get_property_from_name(class_type, property_name.c_str());
  const auto getter_method = mono_property_get_get_method(property);
  const auto string        = reinterpret_cast<MonoString *>(
      mono_runtime_invoke(getter_method, object, nullptr, nullptr));

  return string != nullptr ? std::string{mono_string_to_utf8(string)} : "";
}

MonoClass *EntityScriptType::get_class(MonoImage         *image,
                                       const std::string &namespace_name,
                                       const std::string &class_name)
{
  const auto mono_class =
      mono_class_from_name(image, namespace_name.c_str(), class_name.c_str());
  if (!mono_class)
  {
    DC_LOG_WARN("No class {} in {} found", class_name, namespace_name);
    return nullptr;
  }

  return mono_class;
}

} // namespace dc
