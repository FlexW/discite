#pragma once

#include "event.hpp"

class Layer
{
public:
  virtual ~Layer() = default;

  virtual void init()                   = 0;
  virtual void shutdown()               = 0;
  virtual void update(float delta_time) = 0;
  virtual void render()                 = 0;

  virtual bool on_event(const Event &event) = 0;
};
