#include "uuid.hpp"

#include <random>

namespace
{

std::random_device                      random_device;
std::mt19937_64                         eng(random_device());
std::uniform_int_distribution<uint64_t> uniform_distribution;

} // namespace

namespace dc
{

Uuid generate_uuid() { return uniform_distribution(eng); }

} // namespace dc
