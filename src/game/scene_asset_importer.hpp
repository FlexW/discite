#pragma once

#include "asset_importer_manager.hpp"

namespace dc
{

void import_scene_asset(const std::filesystem::path &file_path,
                        const std::string           &name);
} // namespace dc
