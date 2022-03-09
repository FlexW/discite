#include "event.hpp"

namespace dc
{

Event::Event(std::uint64_t id) : id_{id} {}

std::uint64_t Event::id() const { return id_; }

} // namespace dc
