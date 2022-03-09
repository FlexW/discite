#pragma once

#include <string>

namespace dc
{

struct NameComponent
{
  std::string name_;

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
