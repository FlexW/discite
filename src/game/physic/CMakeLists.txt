target_sources(game PRIVATE
  physx_debugger.cpp
  physx_sdk.cpp
  physx_contact_listener.cpp
  physic_engine.cpp
  physic_scene.cpp
  physx_helper.cpp
  physic_actor.cpp
  physic_controller.cpp
  physic_system.cpp
  physic_settings.cpp
  physic_collider.cpp
  box_collider.cpp
  sphere_collider.cpp
  capsule_collider.cpp
  )

target_link_libraries(game PUBLIC
  PhysXCharacterKinematic
  PhysXCooking
  PhysXVehicle
  PhysXExtensions
  PhysX
  PhysXCommon
  PhysXPvdSDK
  PhysXFoundation
  )

# Set NDEBUG. Otherwise PhysX complains
target_compile_definitions(game PUBLIC NDEBUG)
