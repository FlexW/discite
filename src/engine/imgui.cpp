#include "imgui.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

bool imgui_input(const std::string &name, glm::vec3 &value)
{
  return ImGui::InputFloat3(name.c_str(), glm::value_ptr(value), "%.2f");
}
