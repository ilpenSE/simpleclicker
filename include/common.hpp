#pragma once
#include <QStringLiteral>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <format>
#include <type_traits>

namespace emoji {
  const auto floppy_disk = QStringLiteral("\U0001F4BE");
  const auto mult_x = QStringLiteral("\u2715");
  const auto pencil = QStringLiteral("\u270E");
  const auto trash = QStringLiteral("\U0001F5D1");
}

enum class MouseButton : int {
  Left = 0, Right, Middle, Count,
};

struct Location {
  int x, y;
};

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

template <typename T>
T fromJsonValue(const QJsonValue& jv, const T& def = T{}) {
  if constexpr (std::is_same_v<T, QString>) return jv.toString(def);
  else if constexpr (std::is_same_v<T, bool>) return jv.toBool(def);
  else if constexpr (std::is_same_v<T, int64_t>) return jv.toInteger(def);
  else if constexpr (std::is_same_v<T, int>) return static_cast<int>(jv.toInteger(def));
  else if constexpr (std::is_same_v<T, double>) return jv.toDouble(def);
  else if constexpr (std::is_same_v<T, Location>) {
    auto arr = jv.toArray(QJsonArray{def.x, def.y});
    return Location{static_cast<int>(arr.at(0).toInteger(def.x)), static_cast<int>(arr.at(1).toInteger(def.y))};
  }
  else if constexpr (std::is_same_v<T, MouseButton>) {
    return to_mouse_button(jv.toString(to_cstr(def)));
  }
  else static_assert(!sizeof(T) && "jsonValue<T>: unsupported type");
}

template <typename T>
QJsonValue toJsonValue(const T& v) {
  if constexpr (std::is_same_v<T, int64_t>) return QJsonValue(static_cast<qint64>(v));
  else if constexpr (std::is_same_v<T, Location>) return QJsonValue(QJsonArray{v.x, v.y});
  else if constexpr (std::is_same_v<T, MouseButton>) return QJsonValue(to_cstr(v));
  else return QJsonValue(v);
}

template<>
struct std::formatter<QString> : std::formatter<std::string> {
  auto format(const QString& qstr, std::format_context& ctx) const {
    return std::formatter<std::string>::format(qstr.toStdString(), ctx);
  }
};
