#include "event_manager.hpp"

namespace dc
{

void EventManager::publish(std::shared_ptr<Event> event)
{
  // queue the event
  events_.push(event);
}

void EventManager::dispatch(std::function<void(const Event &)> dispatch_func)
{
  while (events_.size() > 0)
  {
    // get next event
    auto event = events_.front();
    events_.pop();
    dispatch_func(*event);
  }
}

} // namespace dc
