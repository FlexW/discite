#pragma once

#include <cstdio>

namespace dc
{

struct AudioListenerComponent
{
  bool active_{false};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
