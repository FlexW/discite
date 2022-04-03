#include "physic_engine.hpp"
#include "assert.hpp"
#include "frame_data.hpp"
#include "physx_helper.hpp"

#include <thread>

using namespace physx;

namespace
{

constexpr auto pvd_host = "localhost";

template <typename T> static void px_release(T *obj)
{
  if (obj)
  {
    obj->release();
  }
}

} // namespace

namespace dc
{

PhysicEngine::PhysicEngine()
{
  px_foundation_ =
      PxCreateFoundation(PX_PHYSICS_VERSION, px_allocator_, px_error_callback_);

  // Setup pvd
  pvd_                 = PxCreatePvd(*px_foundation_);
  const auto transport = PxDefaultPvdSocketTransportCreate(pvd_host, 5425, 10);
  pvd_->connect(*transport, PxPvdInstrumentationFlag::eALL);

  px_physics_ = PxCreatePhysics(PX_PHYSICS_VERSION,
                                *px_foundation_,
                                PxTolerancesScale(),
                                true,
                                pvd_);
  px_dispatcher_ =
      PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
}

PhysicEngine::~PhysicEngine()
{
  px_release(px_dispatcher_);
  px_release(px_physics_);
  if (pvd_)
  {
    auto transport = pvd_->getTransport();
    pvd_->release();
    pvd_ = nullptr;
    px_release(transport);
  }
  px_release(px_foundation_);
}

void PhysicEngine::update(float delta_time)
{
  if (!px_scene_)
  {
    return;
  }
  px_scene_->simulate(delta_time);
  px_scene_->fetchResults(true);
}

void PhysicEngine::render(SceneRenderInfo &scene_render_info)
{
  if (!debug_draw_ || !px_scene_)
  {
    return;
  }

  const auto color_to_vec3 = [](PxU32 color) -> glm::vec3
  {
    const auto r = ((color >> 16) & 0xff) / 0xff;
    const auto g = ((color >> 8) & 0xff) / 0xff;
    const auto b = ((color >> 0) & 0xff) / 0xff;

    return {r, g, b};
  };

  const auto &px_debug_render_buffer = px_scene_->getRenderBuffer();
  for (PxU32 i = 0; i < px_debug_render_buffer.getNbLines(); ++i)
  {
    const auto &line = px_debug_render_buffer.getLines()[i];

    DebugLineInfo debug_line_info{std::move(to_glm(line.pos0)),
                                  std::move(to_glm(line.pos1)),
                                  std::move(color_to_vec3(line.color0)),
                                  std::move(color_to_vec3(line.color1))};
    scene_render_info.add_debug_line(std::move(debug_line_info));
  }
}

void PhysicEngine::set_debug_draw(bool value) { debug_draw_ = value; }

bool PhysicEngine::debug_draw() const { return debug_draw_; }

void PhysicEngine::create_scene()
{
  PxSceneDesc scene_desc{px_physics_->getTolerancesScale()};
  scene_desc.gravity                 = PxVec3{0.0f, -9.81f, 0.0f};
  scene_desc.cpuDispatcher           = px_dispatcher_;
  scene_desc.filterShader            = PxDefaultSimulationFilterShader;
  scene_desc.kineKineFilteringMode   = PxPairFilteringMode::eKEEP;
  scene_desc.staticKineFilteringMode = PxPairFilteringMode::eKEEP;
  px_scene_                          = px_physics_->createScene(scene_desc);

  const auto pvd_client = px_scene_->getScenePvdClient();
  if (pvd_client)
  {
    pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
    pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
  }

  px_controller_manager_ = PxCreateControllerManager(*px_scene_);

  setup_debug_draw_for_scene();
}

void PhysicEngine::setup_debug_draw_for_scene()
{
  DC_ASSERT(!px_scene_, "Scene already loaded");
  if (!px_scene_)
  {
    return;
  }

  const auto scale_value      = debug_draw_ ? 1.0f : 0.0f;
  const auto actor_axes_value = debug_draw_ ? 2.0f : 0.0f;

  px_scene_->setVisualizationParameter(PxVisualizationParameter::eSCALE,
                                       scale_value);
  px_scene_->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES,
                                       actor_axes_value);
  px_scene_->setVisualizationParameter(
      PxVisualizationParameter::eCOLLISION_SHAPES,
      scale_value);
}

void PhysicEngine::destroy_scene()
{
  DC_ASSERT(px_scene_, "Scene not loaded");
  DC_ASSERT(px_controller_manager_, "Controller manager not loaded");

  px_release(px_controller_manager_);
  px_controller_manager_ = nullptr;
  px_release(px_scene_);
  px_scene_ = nullptr;
}

} // namespace dc
