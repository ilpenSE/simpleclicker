#pragma once

#include <cstddef>
#include <QMap>
#include <QFile>

enum class MouseButton {
  Left, Right, Middle, Count,
};

struct Location {
  int x, y;
};

struct PresetConfig {
  size_t interval;
  Location loc; // {-1, -1} = current location
  MouseButton ms_btn;
  int repeat_cnt; // -1 = repeat until stopped
};

/*
[PresetName]
Interval=400 -> Click Interval in milliseconds
Location=40,10 -> Location to click, "-1,-1" means current location
MouseButton=Left -> can be Left, Right or Middle
Repeat=10 -> Click repeat count (-1 = repeat until stopped)
*/

class PresetManager {
public:
  static PresetManager& instance(const char *file_path) {
    static PresetManager inst(file_path);
    return inst;
  }

  bool init();
  bool serialize();

  PresetConfig get(const char *name) {
    return m_ps.value(name);
  }

  auto append(const char *name, PresetConfig cfg) {
    return m_ps.insert(QString(name), cfg);
  }

  PresetManager(const PresetManager&) = delete;
  PresetManager& operator =(const PresetManager&) = delete;
  PresetManager(PresetManager&&) = delete;
  PresetManager& operator =(PresetManager&&) = delete;
private:
  PresetManager(const char *file_path) : m_file(file_path) {}
  ~PresetManager() {}

  QMap<QString, PresetConfig> m_ps;
  QFile m_file;
};
