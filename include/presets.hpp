#pragma once

#include <cstddef>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QJsonDocument>

enum class MouseButton : int {
  Left = 0, Right, Middle, Count,
};

struct Location {
  int x, y;
};

constexpr auto default_interval = 100;
constexpr auto default_mouse_button = MouseButton::Left;
constexpr auto default_loc = Location{0, 0};
constexpr auto default_repeat = 0;
constexpr auto default_current_loc = true;
constexpr auto default_repeat_forever = true;

constexpr const char *to_cstr(MouseButton msbtn) {
  switch (msbtn) {
  case MouseButton::Left: return "left";
  case MouseButton::Right: return "right";
  case MouseButton::Middle: return "middle";
  default: return nullptr;
  }
}

constexpr MouseButton to_mouse_button(std::string_view str) {
  if (str == "left") return MouseButton::Left;
  if (str == "right") return MouseButton::Right;
  if (str == "middle") return MouseButton::Middle;
  return MouseButton::Left;
}

inline MouseButton to_mouse_button(const QString& qstr) {
  if (qstr == "left") return MouseButton::Left;
  if (qstr == "right") return MouseButton::Right;
  if (qstr == "middle") return MouseButton::Middle;
  return MouseButton::Left;
}

struct PresetConfig {
  size_t interval = default_interval;
  Location loc = default_loc; // {-1, -1} = current location
  MouseButton mouse = default_mouse_button;
  int repeat = default_repeat; // -1 = repeat until stopped
  bool current_loc = default_current_loc;
  bool repeat_forever = default_repeat_forever;
};

class PresetManager {
public:
  static PresetManager& instance(const QString& file_path) {
    static PresetManager inst(file_path);
    return inst;
  }

  bool load();
  void save();
  struct PresetConfig get(const QString& name) const;
  void set(const QString& name, const PresetConfig& cfg);
  void insert(const QString& name, const PresetConfig& cfg);
  struct PresetConfig take(const QString& name);
  auto keyValueRange() const { return m_presets.asKeyValueRange(); }

  PresetManager(const PresetManager&) = delete;
  PresetManager& operator =(const PresetManager&) = delete;
  PresetManager(PresetManager&&) = delete;
  PresetManager& operator =(PresetManager&&) = delete;
private:
  PresetManager(const QString& file_path);
  ~PresetManager() {
    m_file.close();
  }

  QFile m_file{};
  QMap<QString, PresetConfig> m_presets{};
};
