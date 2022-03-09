#pragma once

#include "entity.hpp"
#include "uuid.hpp"

namespace dc
{

class Scene;

struct RelationshipComponent
{
  Entity parent_;
  Entity first_child_;

  Entity next_sibling_;
  Entity previous_sibling_;

  void save(FILE *file) const;
  void read(FILE *file, Scene &active_scene);
};

} // namespace dc
