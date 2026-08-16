#pragma once

#include <cstddef>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QJsonDocument>

enum class MouseButton {
  Left, Right, Middle, Count,
};

struct Location {
  int x, y;
};

constexpr auto default_interval = 100;
constexpr auto default_mouse_button_str = "left";
constexpr auto default_mouse_button = MouseButton::Left;
constexpr auto default_loc = Location{-1, -1};
constexpr auto default_repeat = -1;
constexpr auto default_current_loc = true;

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
};

class PresetManager {
public:
  static PresetManager& instance(const QString& file_path) {
    static PresetManager inst(file_path);
    return inst;
  }

  bool deserialize();
  bool serialize();

  PresetManager(const PresetManager&) = delete;
  PresetManager& operator =(const PresetManager&) = delete;
  PresetManager(PresetManager&&) = delete;
  PresetManager& operator =(PresetManager&&) = delete;

  QMap<QString, PresetConfig> map{};
private:
  PresetManager(const QString& file_path);
  ~PresetManager() {
    m_file.close();
  }

  QFile m_file;
  QJsonDocument m_doc;
};
