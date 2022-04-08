#include "cooking_factory.hpp"
#include "assert.hpp"
#include "asset_description.hpp"
#include "cooking/PxTriangleMeshDesc.h"
#include "engine.hpp"
#include "extensions/PxDefaultStreams.h"
#include "log.hpp"
#include "mesh.hpp"
#include "mesh_asset.hpp"
#include "physic/box_collider_component.hpp"
#include "physic/mesh_collider_data.hpp"
#include "physic/physx_helper.hpp"

#include <PxPhysicsVersion.h>
#include <cooking/PxCooking.h>

#include <filesystem>
#include <memory>
#include <stdexcept>

namespace dc
{

CookingFactory *CookingFactory::get_instance()
{
  static std::unique_ptr<CookingFactory> instance(new CookingFactory);
  return instance.get();
}

void CookingFactory::init(physx::PxFoundation            &foundation,
                          const physx::PxTolerancesScale &scale)
{
  cooking_params_ = std::make_unique<physx::PxCookingParams>(scale);
  cooking_params_->meshWeldTolerance = 0.1f;
  cooking_params_->meshPreprocessParams =
      physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
  cooking_params_->midphaseDesc = physx::PxMeshMidPhase::eBVH34;

  cooking_sdk_ =
      PxCreateCooking(PX_PHYSICS_VERSION, foundation, *cooking_params_);
  DC_ASSERT(cooking_sdk_, "Could not init cooking sdk");
}

void CookingFactory::shutdown()
{
  px_release(cooking_sdk_);
  cooking_sdk_ = nullptr;
}

void CookingFactory::cook_mesh(const MeshAssetHandle &mesh)
{
  DC_ASSERT(mesh.is_ready() && mesh.get(), "Mesh not ready");
  const auto &collider_data = cook_triangle_mesh(*mesh.get());

  const auto &base_directory = Engine::instance()->base_directory();
  const auto  mesh_name      = std::filesystem::path{mesh.asset().id()}.stem().string();
  std::filesystem::create_directories(base_directory / "meshes");
  AssetDescription asset_description{};
  collider_data.save(base_directory / "meshes" / (mesh_name + ".dcpmesh"), asset_description);
}

MeshColliderData CookingFactory::cook_triangle_mesh(const Mesh &mesh)
{
  MeshColliderData collider_data;

  const auto &mesh_desc = mesh.get_description();
  for (const auto &sub_mesh_desc : mesh_desc.sub_meshes_)
  {

    physx::PxTriangleMeshDesc px_mesh_desc{};
    px_mesh_desc.points.stride    = sizeof(Vertex);
    px_mesh_desc.points.count     = sub_mesh_desc.vertices_.size();
    px_mesh_desc.points.data      = sub_mesh_desc.vertices_.data();
    px_mesh_desc.triangles.stride = sizeof(unsigned);
    px_mesh_desc.triangles.count  = sub_mesh_desc.indices_.size() / 3;
    px_mesh_desc.triangles.data   = sub_mesh_desc.indices_.data();

    physx::PxDefaultMemoryOutputStream       buf{};
    physx::PxTriangleMeshCookingResult::Enum result{};
    if (!cooking_sdk_->cookTriangleMesh(px_mesh_desc, buf, &result))
    {
      DC_LOG_WARN("Failed to cook mesh");
      throw std::runtime_error("Failed to cook mesh");
    }

    auto &sub_mesh_data = collider_data.sub_meshes_.emplace_back();

    sub_mesh_data.data_.resize(buf.getSize());
    std::memcpy(sub_mesh_data.data_.data(),
                buf.getData(),
                sub_mesh_data.data_.size());
    sub_mesh_data.transform_ = glm::mat4{1.0f};
  }

  return collider_data;
}

} // namespace dc
