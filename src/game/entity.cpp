#include "entity.hpp"
#include "guid_component.hpp"
#include "math.hpp"
#include "name_component.hpp"
#include "relationship_component.hpp"
#include "transform_component.hpp"

namespace
{

void update_transform_matrix(Entity entity, const glm::mat4 &parent_transform)
{
  while (true)
  {
    if (!entity.valid())
    {
      break;
    }

    // update entity transform and transform of childrens
    auto &transform_component = entity.component<TransformComponent>();
    transform_component.set_parent_transform_matrix(parent_transform);
    auto &relationship_component = entity.component<RelationshipComponent>();
    auto  entity_child           = relationship_component.first_child_;
    update_transform_matrix(entity_child, entity.transform_matrix());

    // update next siblings
    entity = relationship_component.next_sibling_;
  }
}

} // namespace

Entity::Entity(entt::entity entity_handle, std::weak_ptr<Scene> scene)
    : entity_handle_{entity_handle},
      scene_{scene}
{
}

void Entity::set_id(Uuid uuid)
{
  if (valid())
  {
    auto &uuid_component = component<GuidComponent>();
    uuid_component.id_   = uuid;
  }
}

std::uint64_t Entity::id() const
{
  if (valid())
  {
    return component<GuidComponent>().id_;
  }
  return 0;
}

std::string Entity::name() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return {};
  }

  const auto name_component =
      scene->registry_.get<NameComponent>(entity_handle_);

  return name_component.name_;
}

void Entity::set_name(const std::string &name)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &name_component = scene->registry_.get<NameComponent>(entity_handle_);
  name_component.name_ = name;
}

entt::entity Entity::entity_handle() const { return entity_handle_; }

glm::mat4 Entity::transform_matrix() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return glm::mat4{1.0f};
  }

  const auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  return transform_component.transform_matrix();
}

glm::mat4 Entity::local_transform_matrix() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return glm::mat4{1.0f};
  }

  const auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  return transform_component.local_transform_matrix();
}

void Entity::set_local_transform_matrix(const glm::mat4 &transform)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  glm::vec3 positon{};
  glm::vec3 rotation{};
  glm::vec3 scale{};
  if (math::decompose_transform(transform, positon, rotation, scale))
  {
    auto &transform_component =
        scene->registry_.get<TransformComponent>(entity_handle_);
    const auto original_rotation = transform_component.rotation();
    const auto delta_rotation    = rotation - original_rotation;
    set_position(positon);
    set_rotation(original_rotation + delta_rotation);
    set_scale(scale);
  }
}

void Entity::set_position(const glm::vec3 &positon)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  transform_component.set_position(positon);

  auto &relationship_component =
      scene->registry_.get<RelationshipComponent>(entity_handle_);
  update_transform_matrix(relationship_component.first_child_,
                          transform_component.transform_matrix());
}

glm::vec3 Entity::position() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return {};
  }
  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  return transform_component.position();
}

void Entity::set_rotation(const glm::vec3 &rotation)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  transform_component.set_rotation(rotation);

  auto &relationship_component =
      scene->registry_.get<RelationshipComponent>(entity_handle_);
  update_transform_matrix(relationship_component.first_child_,
                          transform_component.transform_matrix());
}

glm::vec3 Entity::rotation() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return {};
  }
  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  return transform_component.rotation();
}

void Entity::set_scale(const glm::vec3 &scale)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  transform_component.set_scale(scale);

  auto &relationship_component =
      scene->registry_.get<RelationshipComponent>(entity_handle_);
  update_transform_matrix(relationship_component.first_child_,
                          transform_component.transform_matrix());
}

glm::vec3 Entity::scale() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return {};
  }
  auto &transform_component =
      scene->registry_.get<TransformComponent>(entity_handle_);
  return transform_component.scale();
}

void Entity::add_child(Entity child)
{
  auto &child_relationship_component = child.component<RelationshipComponent>();

  // remove child from old siblings or parent
  {
    auto next_sibling = child_relationship_component.next_sibling_;
    auto prev_sibling = child_relationship_component.previous_sibling_;
    if (prev_sibling.valid() && next_sibling.valid())
    {
      // child was in the middle of other siblings
      auto &next_sibling_rc = child_relationship_component.next_sibling_
                                  .component<RelationshipComponent>();
      auto &prev_sibling_rc = child_relationship_component.previous_sibling_
                                  .component<RelationshipComponent>();
      prev_sibling_rc.next_sibling_     = next_sibling;
      next_sibling_rc.previous_sibling_ = prev_sibling;
    }
    else if (prev_sibling.valid() && !next_sibling.valid())
    {
      // child was at the end of sibling chain
      auto &prev_sibling_rc = child_relationship_component.previous_sibling_
                                  .component<RelationshipComponent>();
      prev_sibling_rc.next_sibling_ = {};
    }
    else if (!prev_sibling.valid() && next_sibling.valid())
    {
      // child is the first sibling in the chain and has a following sibling
      auto &parent_rc = child_relationship_component.parent_
                            .component<RelationshipComponent>();
      auto &next_sibling_rc = child_relationship_component.next_sibling_
                                  .component<RelationshipComponent>();
      parent_rc.first_child_            = next_sibling;
      next_sibling_rc.previous_sibling_ = {};
    }
    else if (!prev_sibling.valid() && !next_sibling.valid())
    {
      // child is the first sibling and has no siblings
      if (child_relationship_component.parent_.valid())
      {
        // only remove if there is a parent
        auto &parent_rc = child_relationship_component.parent_
                              .component<RelationshipComponent>();
        parent_rc.first_child_ = {};
      }
    }
    else
    {
      // should not happen
      assert(0);
    }
  }

  // insert child in this entity as child
  {
    auto &relationship_component         = component<RelationshipComponent>();
    child_relationship_component.parent_ = *this;
    if (!relationship_component.first_child_.valid())
    {
      // child is the first child of entity
      relationship_component.first_child_            = child;
      child_relationship_component.previous_sibling_ = {};
      child_relationship_component.next_sibling_     = {};

      // update transforms
      update_transform_matrix(child, transform_matrix());
      return;
    }

    auto sibling = relationship_component.first_child_;
    while (true)
    {
      auto &sibling_relationship_component =
          sibling.component<RelationshipComponent>();
      if (!sibling_relationship_component.next_sibling_.valid())
      {
        // found the last sibling in the sibling chain
        sibling_relationship_component.next_sibling_   = child;
        child_relationship_component.previous_sibling_ = sibling;

        // update transforms
        update_transform_matrix(child, transform_matrix());
        return;
      }

      sibling = sibling_relationship_component.next_sibling_;
    }
  }

  // child should be inserted by now. we should never come to this point
  assert(0);
}

void Entity::set_parent(Entity parent) { parent.add_child(*this); }

Entity Entity::parent() const
{
  const auto &relationship_component = component<RelationshipComponent>();
  return relationship_component.parent_;
}

bool Entity::has_parent() const { return parent().valid(); }

bool Entity::operator==(const Entity &other) const
{
  auto scene       = scene_.lock();
  auto other_scene = other.scene_.lock();

  return other.entity_handle_ == entity_handle_ && scene == other_scene;
}

bool Entity::has_childs() const
{
  return component<RelationshipComponent>().first_child_.valid();
}

bool Entity::operator!=(const Entity &other) const { return !(*this == other); }

bool Entity::valid() const { return entity_handle_ != entt::null; }
