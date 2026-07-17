#pragma once

#include "types.h"
#include <filesystem>

namespace backups {

Config load_config(const std::filesystem::path& path);

} // namespace backups
