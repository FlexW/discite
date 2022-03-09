#pragma once

#include <cstdint>

namespace dc
{

using EventId = std::uint64_t;

class Event
{
public:
  Event(EventId id);
  virtual ~Event() = default;

  EventId id() const;

private:
  EventId id_{};
};

} // namespace dc
