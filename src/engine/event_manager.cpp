#include "event_manager.hpp"

namespace dc
{

EventManager::EventManager(EventDispatcher dispatcher)
    : dispatcher_{std::move(dispatcher)}
{
}

void EventManager::fire_event(const Event &event) { dispatcher_(event); }

void EventManager::queue_event(std::shared_ptr<Event> event)
{
  // queue the event
  events_.push(event);
}

void EventManager::dispatch_events()
{
  while (events_.size() > 0)
  {
    // get next event
    auto event = events_.front();
    events_.pop();
    dispatcher_(*event);
  }
}

} // namespace dc
