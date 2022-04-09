cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  "Discite Engine"
  VERSION 0.0.1
  LANGUAGES CXX C
  )

option(USE_WAYLAND "Compile for Wayland" OFF)

# Include modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Set binary output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${DC_BIN_DIR})

# It's not optimal that the file gets created in the tree.
# However, I have no better idea
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/script_runtime/DisciteRuntime.csproj.in
  ${CMAKE_CURRENT_SOURCE_DIR}/script_runtime/DisciteRuntime.csproj)

# Colored compiler output
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

include(warnings)
include(FeatureSummary)

message(STATUS "Bootstrap dependencies")
execute_process(
  COMMAND "python" "${CMAKE_SOURCE_DIR}/bootstrap.py"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(external)
add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES INCLUDE_QUIET_PACKAGES)
