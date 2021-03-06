add_subdirectory(glad EXCLUDE_FROM_ALL)
add_subdirectory(deps/src/glm EXCLUDE_FROM_ALL)
add_subdirectory(stb EXCLUDE_FROM_ALL)
add_subdirectory(fastdelegate EXCLUDE_FROM_ALL)

# GLFW
if (USE_WAYLAND)
  set(GLFW_USE_WAYLAND "ON" CACHE STRING "")
else()
  set(GLFW_USE_WAYLAND "OFF" CACHE STRING "")
endif()
set(GLFW_BUILD_EXAMPLES "OFF" CACHE STRING "")
set(GLFW_BUILD_TESTS "OFF" CACHE STRING "")
set(GLFW_BUILD_DOCS "OFF" CACHE STRING "")
set(GLFW_INSTALL "OFF" CACHE STRING "")
set(BUILD_SHARED_LIBS "OFF" CACHE STRING "")
add_subdirectory(deps/src/glfw EXCLUDE_FROM_ALL)

# Assimp
set(ASSIMP_BUILD_ZLIB "ON" CACHE STRING "")
set(ASSIMP_BUILD_ASSIMP_TOOLS "OFF" CACHE STRING "")
set(ASSIMP_BUILD_TESTS "OFF" CACHE STRING "")
set(ASSIMP_BUILD_TOOLS "OFF" CACHE STRING "")
set(ASSIMP_INSTALL "OFF" CACHE STRING "")
set(BUILD_SHARED_LIBS "OFF" CACHE STRING "")
set(ASSIMP_NO_EXPORT "ON" CACHE STRING "")
set(ASSIMP_BUILD_ASE_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_AMF_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_3DS_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_AC_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_AE_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_ASSBIN_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_B3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_BVH_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_DXF_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_CSM_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_HMP_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_IRRMESH_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_IQM_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_IRR_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_LWO_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_LWS_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_M3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MD2_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MD3_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MD5_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MDC_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MDL_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_NFF_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_NDO_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_OFF_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_OGRE_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_OPENGEX_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_PLY_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MS3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_COB_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_BLEND_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_IFC_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_XGL_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_FBX_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_Q3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_Q3BSP_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_RAW_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_SIB_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_SMD_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_STL_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_TERRAGEN_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_X_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_X3D_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_3MF_IMPORTER "OFF" CACHE STRING "")
set(ASSIMP_BUILD_MMD_IMPORTER "OFF" CACHE STRING "")
add_subdirectory(deps/src/assimp EXCLUDE_FROM_ALL)

# ImGui
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(imgui STATIC)
target_link_libraries(imgui PUBLIC glfw)
target_sources(imgui PRIVATE
  deps/src/imgui/imgui.cpp
  deps/src/imgui/imgui_demo.cpp
  deps/src/imgui/imgui_draw.cpp
  deps/src/imgui/imgui_tables.cpp
  deps/src/imgui/imgui_widgets.cpp

  deps/src/imgui/backends/imgui_impl_glfw.cpp
  deps/src/imgui/backends/imgui_impl_opengl3.cpp
  )
target_include_directories(imgui PUBLIC deps/src/imgui deps/src/imgui/backends)

# ImGuizmo
add_library(imguizmo STATIC)
target_sources(imguizmo PRIVATE
  deps/src/imguizmo/ImGuizmo.cpp
  )
target_include_directories(imguizmo PUBLIC deps/src/imgui deps/src/imguizmo)

# EnTT
add_subdirectory(deps/src/entt EXCLUDE_FROM_ALL)

# gli
set(GLI_TEST_ENABLE "OFF" CACHE STRING "")
add_subdirectory(deps/src/gli EXCLUDE_FROM_ALL)

# RapidJSON
set(RAPIDJSON_BUILD_DOC "OFF" CACHE STRING "")
set(RAPIDJSON_BUILD_EXAMPLES "OFF" CACHE STRING "")
set(RAPIDJSON_BUILD_TESTS "OFF" CACHE STRING "")
set(RAPIDJSON_BUILD_CXX11 "OFF" CACHE STRING "")
set(RAPIDJSON_BUILD_CXX17 "ON" CACHE STRING "")
add_library(rapidjson INTERFACE)
set_property(TARGET rapidjson PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/rapidjson/include)

# Fmt
add_subdirectory(deps/src/fmt EXCLUDE_FROM_ALL)

# spdlog
set(SPDLOG_FMT_EXTERNAL "ON" CACHE STRING "")
add_subdirectory(deps/src/spdlog EXCLUDE_FROM_ALL)

# Optick
add_subdirectory(deps/src/optick EXCLUDE_FROM_ALL)

# PhysX
set(TARGET_BUILD_PLATFORM "linux")
set(PX_GENERATE_STATIC_LIBRARIES "ON" CACHE STRING "")
set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/deps/src/physx/physx)
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION "1.27")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/deps/src/physx/physx)
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/deps/src/physx/physx)
add_subdirectory(deps/src/physx/physx/source/compiler/cmake EXCLUDE_FROM_ALL)

# OpenAL
set(LIBTYPE "STATIC" CACHE STRING "")
set(ALSOFT_UTILS "OFF" CACHE STRING "")
set(ALSOFT_NO_CONFIG_UTIL "ON" CACHE STRING "")
set(ALSOFT_EXAMPLES "OFF" CACHE STRING "")
set(ALSOFT_INSTALL "OFF" CACHE STRING "")
set(ALSOFT_INSTALL_CONFIG "OFF" CACHE STRING "")
set(ALSOFT_INSTALL_HRTF_DATA "OFF" CACHE STRING "")
set(ALSOFT_INSTALL_AMBDEC_PRESETS "OFF" CACHE STRING "")
set(ALSOFT_INSTALL_EXAMPLES "OFF" CACHE STRING "")
set(ALSOFT_INSTALL_UTILS "OFF" CACHE STRING "")
add_subdirectory(deps/src/openal EXCLUDE_FROM_ALL)
