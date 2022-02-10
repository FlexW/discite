#pragma once

class Game
{
public:
  virtual ~Game() = default;

  virtual void init()                   = 0;
  virtual void update(float delta_time) = 0;
  virtual void render()                 = 0;
  virtual void render_imgui()           = 0;
};
