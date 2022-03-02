#include "relationship_component.hpp"
#include "scene.hpp"

void RelationshipComponent::save(FILE *file) const
{
  write_value(file, parent_.id());
  write_value(file, first_child_.id());
  write_value(file, next_sibling_.id());
  write_value(file, previous_sibling_.id());
}

void RelationshipComponent::read(FILE *file, Scene &active_scene)
{
  Uuid parent_id{0};
  read_value(file, parent_id);
  parent_ = active_scene.get_or_create_entity(parent_id);

  Uuid first_child_id{0};
  read_value(file, first_child_id);
  first_child_ = active_scene.get_or_create_entity(first_child_id);

  Uuid next_sibling_id{0};
  read_value(file, next_sibling_id);
  next_sibling_ = active_scene.get_or_create_entity(next_sibling_id);

  Uuid previous_sibling_id{0};
  read_value(file, previous_sibling_id);
  previous_sibling_ = active_scene.get_or_create_entity(previous_sibling_id);
}
