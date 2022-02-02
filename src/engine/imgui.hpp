#pragma once

#include "math.hpp"

#include <imgui.h>

bool imgui_input(const std::string &name, glm::vec3 &value);

bool imgui_input(const std::string &name, float &value);
