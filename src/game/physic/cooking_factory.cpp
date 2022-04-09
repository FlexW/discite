#include "cooking_factory.hpp"
#include "assert.hpp"
#include "asset_description.hpp"
#include "box_collider_component.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "mesh_asset.hpp"
#include "mesh_collider_data.hpp"
#include "physx_helper.hpp"

#include <PxPhysicsVersion.h>
#include <cooking/PxConvexMeshDesc.h>
#include <cooking/PxCooking.h>
#include <cooking/PxTriangleMeshDesc.h>
#include <cstdint>
#include <extensions/PxDefaultStreams.h>

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

std::shared_ptr<MeshColliderData>
CookingFactory::cook_mesh(const MeshAssetHandle &mesh,
                          MeshColliderType       mesh_collider_type,
                          bool                   invalidate)
{

  const auto mesh_name =
      std::filesystem::path{mesh.asset().id()}.stem().string();
  const auto asset_name =
      (mesh_name + (mesh_collider_type == MeshColliderType::Convex
                        ? ".dcpcmesh"
                        : ".dcptmesh"));
  const auto &base_directory = Engine::instance()->base_directory();
  const auto  file_path      = base_directory / "meshes" / asset_name;

  const auto mesh_iter = mesh_cache_.find(file_path.string());
  if (!invalidate && mesh_iter != mesh_cache_.end())
  {
    return mesh_iter->second;
  }
  else if (std::filesystem::exists(file_path))
  {
    const auto collider_data = std::make_shared<MeshColliderData>();
    collider_data->read(file_path);
    mesh_cache_[file_path.string()] = collider_data;
    return collider_data;
  }

  DC_ASSERT(mesh.is_ready() && mesh.get(), "Mesh not ready");

  std::shared_ptr<MeshColliderData> collider_data{};
  if (mesh_collider_type == MeshColliderType::Convex)
  {
    collider_data = cook_convex_mesh(*mesh.get());
  }
  else
  {
    collider_data = cook_triangle_mesh(*mesh.get());
  }

  std::filesystem::create_directories(base_directory / "meshes");
  AssetDescription asset_description{};
  collider_data->save(file_path, asset_description);

  mesh_cache_[file_path.string()] = collider_data;

  return collider_data;
}

std::shared_ptr<MeshColliderData>
CookingFactory::cook_triangle_mesh(const Mesh &mesh)
{
  const auto collider_data      = std::make_shared<MeshColliderData>();
  collider_data->collider_type_ = MeshColliderType::Triangle;

  const auto &mesh_desc = mesh.get_description();
  for (const auto &sub_mesh_desc : mesh_desc.sub_meshes_)
  {

    physx::PxTriangleMeshDesc px_mesh_desc{};
    px_mesh_desc.points.data      = sub_mesh_desc.vertices_.data();
    px_mesh_desc.points.count     = sub_mesh_desc.vertices_.size();
    px_mesh_desc.points.stride    = sizeof(Vertex);

    px_mesh_desc.triangles.data   = sub_mesh_desc.indices_.data();
    px_mesh_desc.triangles.count  = sub_mesh_desc.indices_.size() / 3;
    px_mesh_desc.triangles.stride = sizeof(std::uint32_t) * 3;

    DC_ASSERT(px_mesh_desc.isValid(),
              "Invalid physx triangle mesh description");

    physx::PxDefaultMemoryOutputStream       buf{};
    physx::PxTriangleMeshCookingResult::Enum result{};
    if (!cooking_sdk_->cookTriangleMesh(px_mesh_desc, buf, &result))
    {
      DC_LOG_WARN("Failed to triangle mesh");
      throw std::runtime_error("Failed to cook triangle mesh");
    }

    auto &sub_mesh_data = collider_data->sub_meshes_.emplace_back();

    sub_mesh_data.data_.resize(buf.getSize());
    std::memcpy(sub_mesh_data.data_.data(),
                buf.getData(),
                sub_mesh_data.data_.size());
    sub_mesh_data.transform_ = glm::mat4{1.0f};
  }

  return collider_data;
}

std::shared_ptr<MeshColliderData>
CookingFactory::cook_convex_mesh(const Mesh &mesh)
{
  const auto collider_data      = std::make_shared<MeshColliderData>();
  collider_data->collider_type_ = MeshColliderType::Convex;

  const auto &mesh_desc = mesh.get_description();
  for (const auto &sub_mesh_desc : mesh_desc.sub_meshes_)
  {
    physx::PxConvexMeshDesc px_mesh_desc{};

    px_mesh_desc.points.data   = sub_mesh_desc.vertices_.data();
    px_mesh_desc.points.count  = sub_mesh_desc.vertices_.size();
    px_mesh_desc.points.stride = sizeof(Vertex);

    px_mesh_desc.indices.data   = sub_mesh_desc.indices_.data();
    px_mesh_desc.indices.count  = sub_mesh_desc.indices_.size() / 3;
    px_mesh_desc.indices.stride = sizeof(std::uint32_t) * 3;

    px_mesh_desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX |
                         physx::PxConvexFlag::eSHIFT_VERTICES;

    DC_ASSERT(px_mesh_desc.isValid(), "Invalid physx convex mesh description");

    physx::PxDefaultMemoryOutputStream     buf{};
    physx::PxConvexMeshCookingResult::Enum result{};
    if (!cooking_sdk_->cookConvexMesh(px_mesh_desc, buf, &result))
    {
      DC_LOG_WARN("Failed to cook convex mesh");
      throw std::runtime_error("Failed to cook convex mesh");
    }

    auto &sub_mesh_data = collider_data->sub_meshes_.emplace_back();

    sub_mesh_data.data_.resize(buf.getSize());
    std::memcpy(sub_mesh_data.data_.data(),
                buf.getData(),
                sub_mesh_data.data_.size());
    sub_mesh_data.transform_ = glm::mat4{1.0f};
  }

  return collider_data;
}

} // namespace dc
