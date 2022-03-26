#pragma once

#include "event.hpp"

#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>

namespace dc
{
using EventDispatcher = std::function<void(const Event &)>;

class EventManager
{
public:
  EventManager(EventDispatcher dispatcher);

  /**
   * Fires an event immedentialey to the subscribers
   */
  void fire_event(const Event &event);

  /**
   * Queues an event.
   *
   * The event will get passed to subscribers when calling dispatch_events()
   */
  void queue_event(std::shared_ptr<Event> event);

  void dispatch_events();

private:
  EventDispatcher dispatcher_;

  std::queue<std::shared_ptr<Event>> events_;
};

} // namespace dc
