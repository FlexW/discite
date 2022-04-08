#pragma once

#include "mesh.hpp"
#include "mesh_asset.hpp"
#include "physic/mesh_collider_data.hpp"

#include <PxFoundation.h>
#include <common/PxTolerancesScale.h>
#include <cooking/PxCooking.h>

#include <memory>

namespace dc
{

class CookingFactory
{
public:
  static CookingFactory *get_instance();

  void init(physx::PxFoundation            &foundation,
            const physx::PxTolerancesScale &scale);
  void shutdown();

  void cook_mesh(const MeshAssetHandle &mesh);

private:
  physx::PxCooking      *cooking_sdk_;
  std::unique_ptr<physx::PxCookingParams> cooking_params_;

  CookingFactory() = default;

  MeshColliderData cook_triangle_mesh(const Mesh &mesh);
};

} // namespace dc
