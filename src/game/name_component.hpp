#pragma once

#include <string>

struct NameComponent
{
  std::string name_;

  void save(FILE *file) const;
  void read(FILE *file);
};
