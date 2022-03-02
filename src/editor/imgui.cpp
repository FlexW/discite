#include "imgui.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include <array>
#include <cstring>

bool imgui_input(const std::string  &name,
                 std::string        &value,
                 ImGuiInputTextFlags flags)
{
  std::array<char, 256> input{};

  const auto size = std::min(input.size(), value.size());
  std::memcpy(input.data(), value.data(), size);

  if (ImGui::InputText(name.c_str(), input.data(), input.size(), flags))
  {
    value = input.data();
    return true;
  }

  return false;
}

bool imgui_input(const std::string &name, glm::vec3 &value)
{
  return ImGui::InputFloat3(name.c_str(), glm::value_ptr(value), "%.2f");
}

bool imgui_input(const std::string &name, float &value)
{
  return ImGui::InputFloat(name.c_str(), &value);
}

bool imgui_input(const std::string &name, bool &value)
{
  return ImGui::Checkbox(name.c_str(), &value);
}
