target_sources(game PRIVATE
  physx_debugger.cpp
  physx_sdk.cpp
  physx_contact_listener.cpp
  physic_engine.cpp
  physic_scene.cpp
  physx_helper.cpp
  rigid_body.cpp
  rigid_body_component.cpp
  character_controller.cpp
  character_controller_component.cpp
  physic_system.cpp
  physic_settings.cpp
  physic_collider.cpp
  box_collider.cpp
  box_collider_component.cpp
  sphere_collider.cpp
  sphere_collider_component.cpp
  capsule_collider.cpp
  capsule_collider_component.cpp
  cooking_factory.cpp
  mesh_collider_data.cpp
  mesh_collider.cpp
  mesh_collider_component.cpp
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
