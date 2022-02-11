#pragma once

class ImGuiPanel
{
public:
  virtual ~ImGuiPanel() = default;

  virtual void render() = 0;
};
