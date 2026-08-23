#pragma once
#include <QStringLiteral>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <format>
#include <type_traits>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QPixmap>
#include <QGuiApplication>
#include <QStyleHints>
#include <QIcon>
#include <QPushButton>

constexpr auto SAVE_CHANGES_KEYBIND = "Ctrl+S";
constexpr auto ABORT_CHANGES_KEYBIND = "Ctrl+K";

enum class Theme : int { Dark = 0, Light, Count };

enum class Language : int { English = 0, Turkish, Count };

enum class MouseButton : int {
  Left = 0, Right, Middle, Count,
};

struct Location {
  int x, y;
};

constexpr const char *to_cstr(Theme theme) {
  switch (theme) {
  case Theme::Dark: return "dark";
  case Theme::Light: return "light";
  default: return nullptr;
  }
}

constexpr Theme to_theme(std::string_view sv) {
  if (sv == "dark") return Theme::Dark;
  if (sv == "light") return Theme::Light;
  return Theme::Dark;
}

inline Theme to_theme(const QString &qstr) {
  if (qstr == "dark") return Theme::Dark;
  if (qstr == "light") return Theme::Light;
  return Theme::Dark;
}

constexpr const char *to_cstr(Language lang) {
  switch (lang) {
  case Language::English: return "en";
  case Language::Turkish: return "tr";
  default: return nullptr;
  }
}

constexpr Language to_language(std::string_view sv) {
  if (sv == "en") return Language::English;
  if (sv == "tr") return Language::Turkish;
  return Language::English;
}

inline Language to_language(const QString &qstr) {
  if (qstr == "en") return Language::English;
  if (qstr == "tr") return Language::Turkish;
  return Language::English;
}

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
  else if constexpr (std::is_same_v<T, Theme>) {
    return to_theme(jv.toString(to_cstr(def)));
  }
  else if constexpr (std::is_same_v<T, Language>) {
    return to_language(jv.toString(to_cstr(def)));
  }
  else static_assert(!sizeof(T) && "jsonValue<T>: unsupported type");
}

template <typename T>
QJsonValue toJsonValue(const T& v) {
  if constexpr (std::is_same_v<T, int64_t>) return QJsonValue(static_cast<qint64>(v));
  else if constexpr (std::is_same_v<T, Location>) return QJsonValue(QJsonArray{v.x, v.y});
  else if constexpr (std::is_same_v<T, MouseButton>) return QJsonValue(to_cstr(v));
  else if constexpr (std::is_same_v<T, Theme>) return QJsonValue(to_cstr(v));
  else if constexpr (std::is_same_v<T, Language>) return QJsonValue(to_cstr(v));
  else return QJsonValue(v);
}

template<>
struct std::formatter<QString> : std::formatter<std::string> {
  auto format(const QString& qstr, std::format_context& ctx) const {
    return std::formatter<std::string>::format(qstr.toStdString(), ctx);
  }
};

void makeDynamicIconButton(QPushButton *btn, const QString& symbol);
