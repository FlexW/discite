#pragma once

#include "uuid.hpp"

#include <cstdint>
#include <stdio.h>

namespace dc
{

struct GuidComponent
{
  GuidComponent();
  explicit GuidComponent(Uuid uuid);

  Uuid id_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
