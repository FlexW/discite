#pragma once

#include "event.hpp"

#include <functional>
#include <memory>
#include <queue>

namespace dc
{

class EventManager
{
public:
  void publish(std::shared_ptr<Event> event);
  void dispatch(std::function<void(const Event &)> dispatch_func);

private:
  std::queue<std::shared_ptr<Event>> events_;
};

} // namespace dc
