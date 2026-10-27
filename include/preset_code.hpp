#pragma once

#include <QString>
#include "presets.hpp"

constexpr auto FORMAT_PREFIX = "SC1-";
constexpr auto FORMAT_VERSION = 1;

extern QString preset_code_serialize(const QString &name, const PresetConfig &cfg);
extern std::optional<std::pair<QString, PresetConfig>> preset_code_deserialize(const QString &code);
