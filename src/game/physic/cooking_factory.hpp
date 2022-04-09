#pragma once

#include "mesh.hpp"
#include "mesh_asset.hpp"
#include "physic/mesh_collider_data.hpp"

#include <PxFoundation.h>
#include <common/PxTolerancesScale.h>
#include <cooking/PxCooking.h>

#include <memory>
#include <unordered_map>

namespace dc
{

class CookingFactory
{
public:
  static CookingFactory *get_instance();

  void init(physx::PxFoundation            &foundation,
            const physx::PxTolerancesScale &scale);
  void shutdown();

  std::shared_ptr<MeshColliderData>
  cook_mesh(const MeshAssetHandle &mesh,
            MeshColliderType       mesh_collider_type,
            bool                   invalidate = false);

private:
  physx::PxCooking      *cooking_sdk_;
  std::unique_ptr<physx::PxCookingParams> cooking_params_;

  std::unordered_map<std::string, std::shared_ptr<MeshColliderData>>
      mesh_cache_;

  CookingFactory() = default;

  std::shared_ptr<MeshColliderData> cook_triangle_mesh(const Mesh &mesh);
  std::shared_ptr<MeshColliderData> cook_convex_mesh(const Mesh &mesh);
};

} // namespace dc
