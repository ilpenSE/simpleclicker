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
#include <QMessageBox>

namespace {
// string -> int converter only for version
constexpr bool parse_uint8_version(std::string_view sv, uint8_t& out) {
  if (sv.empty() || sv.size() > 3) return false;

  unsigned int val = 0;
  for (char c : sv) {
    if (c < '0' || c > '9') return false;
    val = val * 10 + static_cast<unsigned int>(c - '0');
    if (val > 255) return false; // overflow guard
  }

  out = static_cast<uint8_t>(val);
  return true;
}
} // namespace

struct Hotkey {
  Qt::Key key = Qt::Key_unknown;
  bool ctrl = false;
  bool shift = false;
  bool alt = false;

  bool operator==(const Hotkey &other) const {
    return key == other.key && ctrl == other.ctrl
           && shift == other.shift && alt == other.alt;
  }
  bool operator!=(const Hotkey &other) const {
    return !(*this == other);
  }

  QString toString() const {
    if (!isValid()) return "";
    int combined = key;
    if (ctrl)  combined |= Qt::CTRL;
    if (shift) combined |= Qt::SHIFT;
    if (alt)   combined |= Qt::ALT;
    return QKeySequence(combined).toString(QKeySequence::PortableText);
  }

  QKeySequence toKeySequence() const {
    if (!isValid()) return {};
    int combined = key;
    if (ctrl)  combined |= Qt::CTRL;
    if (shift) combined |= Qt::SHIFT;
    if (alt) combined |= Qt::ALT;
    return combined;
  }

  static Hotkey from(const QKeySequence& seq) {
    if (seq.isEmpty()) return {};
    Hotkey hk;
    int combined = seq[0].toCombined();
    hk.ctrl  = (combined & Qt::CTRL)  != 0;
    hk.shift = (combined & Qt::SHIFT) != 0;
    hk.alt   = (combined & Qt::ALT)   != 0;
    hk.key = static_cast<Qt::Key>(combined &
                                  ~(Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::META));
    return hk;
  }

  static Hotkey from(const QString& str) {
    if (str.isEmpty()) return {};
    QKeySequence seq(str, QKeySequence::PortableText);
    if (seq.isEmpty()) return {}; // parse error
    Hotkey hk;
    int combined = seq[0].toCombined();
    hk.ctrl  = (combined & Qt::CTRL)  != 0;
    hk.shift = (combined & Qt::SHIFT) != 0;
    hk.alt   = (combined & Qt::ALT)   != 0;
    hk.key = static_cast<Qt::Key>(combined &
                                  ~(Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::META));
    return hk;
  }

  constexpr bool isValid() const { return key != Qt::Key_unknown; }
};

constexpr auto SAVE_CHANGES_KEYBIND = Hotkey{.key=Qt::Key_S, .ctrl=true};
constexpr auto ABORT_CHANGES_KEYBIND = Hotkey{.key=Qt::Key_K, .ctrl=true};

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

inline Language to_language(QLocale::Language locale_lang) {
  switch (locale_lang) {
  case QLocale::Turkish: return Language::Turkish;
  default: return Language::English;
  }
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

constexpr inline Location to_location(QPoint point) {
  return {point.x(), point.y()};
}

struct Version {
  uint8_t major = 0, minor = 0, patch = 0;
  bool is_beta = false;

  QString toQString() const {
    if (is_beta) return QString("%1.%2.%3-beta").arg(major).arg(minor).arg(patch);
    return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
  }

  std::string toString() const {
    if (is_beta) return std::format("{}.{}.{}-beta", major, minor, patch);
    return std::format("{}.{}.{}", major, minor, patch);
  }

  static Version from(QStringView sv);

  static constexpr Version from(std::string_view sv) {
    bool is_beta = sv.ends_with("-beta");
    if (is_beta) sv.remove_suffix(5);

    Version res;
    res.is_beta = is_beta;

    for (int i = 0; i < 3; ++i) {
      size_t dot = sv.find('.');
      std::string_view part = (dot == std::string_view::npos) ? sv : sv.substr(0, dot);

      uint8_t val = 0;
      if (!parse_uint8_version(part, val)) return {};

      if (i == 0) res.major = val;
      else if (i == 1) res.minor = val;
      else res.patch = val;

      if (dot == std::string_view::npos) break;
      sv.remove_prefix(dot + 1);
    }
    return res;
  }
};

constexpr Version APP_VERSION = Version::from("1.0.0");

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
  else if constexpr (std::is_same_v<T, Hotkey>) {
    return Hotkey::from(jv.toString(def.toString()));
  }
  else if constexpr (std::is_same_v<T, Version>) {
    return Version::from(jv.toString(def.toQString()));
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
  else if constexpr (std::is_same_v<T, Hotkey>) return QJsonValue(v.toString());
  else if constexpr (std::is_same_v<T, Version>) return QJsonValue(v.toQString());
  else return QJsonValue(v);
}

template<>
struct std::formatter<QString> : std::formatter<std::string> {
  auto format(const QString& qstr, std::format_context& ctx) const {
    return std::formatter<std::string>::format(qstr.toStdString(), ctx);
  }
};

template<>
struct std::formatter<Language> : std::formatter<std::string> {
  auto format(Language lang, std::format_context& ctx) const {
    return std::formatter<std::string>::format(to_cstr(lang), ctx);
  }
};

template<>
struct std::formatter<Theme> : std::formatter<std::string> {
  auto format(Theme theme, std::format_context& ctx) const {
    return std::formatter<std::string>::format(to_cstr(theme), ctx);
  }
};

template<>
struct std::formatter<Location> : std::formatter<std::string> {
  auto format(Location loc, std::format_context &ctx) const {
    std::string result = "Location{";
    result += std::to_string(loc.x);
    result += ", ";
    result += std::to_string(loc.y);
    result += "}";
    return std::formatter<std::string>::format(result, ctx);
  }
};

template<>
struct std::formatter<Hotkey> : std::formatter<std::string> {
  auto format(const Hotkey& hk, std::format_context &ctx) const {
    return std::formatter<std::string>::format(hk.toString().toStdString(), ctx);
  }
};

template<>
struct std::formatter<MouseButton> : std::formatter<std::string> {
  auto format(MouseButton msbtn, std::format_context &ctx) const {
    return std::formatter<std::string>::format(to_cstr(msbtn), ctx);
  }
};

template<>
struct std::formatter<Version> : std::formatter<std::string> {
  auto format(Version ver, std::format_context &ctx) const {
    return std::formatter<std::string>::format(ver.toString(), ctx);
  }
};

void makeDynamicIconButton(QPushButton *btn, const QString& symbol);

template <typename... Args>
[[noreturn]]
inline void panic(std::format_string<Args...> fmt, Args &&...args) {
  std::string s = std::format<Args...>(fmt, std::forward<Args>(args)...);
  fprintf(stderr, "\e[0;31mPROGRAM PANICKED!\e[0m\n");
  fprintf(stderr, "\e[0;31mFATAL ERROR:\e[0m %.*s\n", s);

  QString title;
  if (qgetenv("LANG").startsWith("tr_TR")) title = "ÖLÜMCÜL HATA";
  else title = "FATAL ERROR";
  QMessageBox::critical(nullptr, std::move(title), QString::fromUtf8(s));

  abort();
}
