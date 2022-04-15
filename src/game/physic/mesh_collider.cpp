#include "mesh_collider.hpp"
#include "assert.hpp"
#include "cooking_factory.hpp"
#include "math.hpp"
#include "mesh_collider_component.hpp"
#include "mesh_collider_data.hpp"
#include "mesh_component.hpp"
#include "physx_helper.hpp"
#include "physx_sdk.hpp"

#include <extensions/PxDefaultStreams.h>
#include <foundation/PxSimpleTypes.h>
#include <geometry/PxMeshScale.h>
#include <geometry/PxTriangleMesh.h>
#include <geometry/PxTriangleMeshGeometry.h>

namespace dc
{

MeshCollider::MeshCollider(Entity               entity,
                           physx::PxRigidActor *physx_actor,
                           MeshColliderType     mesh_collider_type)
    : PhysicCollider{type},
      entity_{entity}
{
  const auto &mesh_component     = entity_.component<MeshComponent>();
  auto       &collider_component = entity_.component<MeshColliderComponent>();
  DC_ASSERT(!collider_component.mesh_collider_, "Mesh collider already set");
  collider_component.mesh_collider_ = this;

  if (mesh_collider_type == MeshColliderType::Convex)
  {
    collider_component.is_convex_ = true;
  }
  else
  {
    collider_component.is_convex_ = false;
  }

  set_physic_material(collider_component.physic_material_);

  const auto collider_data = CookingFactory::get_instance()->cook_mesh(
      *mesh_component.model_,
      collider_component.is_convex_ ? MeshColliderType::Convex
                                    : MeshColliderType::Triangle);

  for (auto &sub_mesh_collider : collider_data->sub_meshes_)
  {
    glm::vec3 translation{}, rotation{}, scale{};
    math::decompose_transform(sub_mesh_collider.transform_,
                              translation,
                              rotation,
                              scale);

    auto &data = sub_mesh_collider.data_;

    physx::PxDefaultMemoryInputData input{
        static_cast<physx::PxU8 *>(data.data()),
        static_cast<physx::PxU32>(data.size())};

    const auto physx = PhysXSdk::get_instance()->get_physics();
    if (mesh_collider_type == MeshColliderType::Convex)
    {
      const auto convex_mesh     = physx->createConvexMesh(input);
      const auto convex_geometry = physx::PxConvexMeshGeometry(
          convex_mesh,
          physx::PxMeshScale(to_physx(scale * entity_.scale())));

      const auto shape =
          physx->createShape(convex_geometry, *get_physic_material(), true);
      shape->setLocalPose(to_physx_transform(translation, rotation));
      shape->userData = this;

      physx_actor->attachShape(*shape);

      shapes_.push_back(shape);

      px_release(convex_mesh);
    }
    else
    {
      const auto triangle_mesh     = physx->createTriangleMesh(input);
      const auto triangle_geometry = physx::PxTriangleMeshGeometry(
          triangle_mesh,
          physx::PxMeshScale(to_physx(scale * entity_.scale())));

      const auto shape =
          physx->createShape(triangle_geometry, *get_physic_material(), true);
      shape->setLocalPose(to_physx_transform(translation, rotation));
      shape->userData = this;

      physx_actor->attachShape(*shape);

      shapes_.push_back(shape);

      px_release(triangle_mesh);
    }
  }
  set_trigger(collider_component.is_trigger_);
}

void MeshCollider::set_trigger(bool value)
{
  for (const auto &shape : shapes_)
  {
    if (value)
    {
      shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
      shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
    }
    else
    {
      shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
      shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
    }
  }

  entity_.component<MeshColliderComponent>().is_trigger_ = value;
}

bool MeshCollider::is_trigger() const
{
  return entity_.component<MeshColliderComponent>().is_trigger_;
}

void MeshCollider::set_offset(const glm::vec3 & /*offset*/) {}

glm::vec3 MeshCollider::get_offset() const { return {}; }

void MeshCollider::detach_from_actor(physx::PxRigidActor &actor)
{
  for (const auto &shape : shapes_)
  {
    actor.detachShape(*shape);
  }
  shapes_.clear();
}

} // namespace dc
