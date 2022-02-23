#pragma once

#include "layer.hpp"

#include <array>

class IrrMapLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;
  void update(float delta_time) override;
  void render() override;

  bool on_event(const Event &event) override;
};
