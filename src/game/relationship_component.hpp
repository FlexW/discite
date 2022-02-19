#pragma once

#include "entity.hpp"

struct RelationshipComponent
{
  Entity parent_;
  Entity first_child_;

  Entity next_sibling_;
  Entity previous_sibling_;
};
