#pragma once

#include <QFile>
#include <format>
#include <QDir>
#include <QTextStream>

#define LOG_LEVELS \
  X(INFO, info) \
  X(ERROR, error) \
  X(WARNING, warning) \
  X(FATAL, fatal)

enum class LogLevel {
#define X(name, _) name,
LOG_LEVELS
#undef X
  Count,
};

inline const char *to_cstr(LogLevel level) {
  switch (level) {
  #define X(name, _) case LogLevel::name: return #name;
  LOG_LEVELS
  #undef X
  default: return nullptr;
  }
}

class Logger {
public:
  static Logger& instance(const QDir& logs_dir) {
    static Logger inst(logs_dir);
    return inst;
  }

  template <typename... Args>
  void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    log_internal(level, formatted);
  }

  #define X(upper, lower)                                                  \
  template <typename... Args>                                              \
  void lower(std::format_string<Args...> fmt, Args&&... args) {            \
    std::string formatted = std::format(fmt, std::forward<Args>(args)...); \
    log_internal(LogLevel::upper, formatted);                              \
  }
  LOG_LEVELS
  #undef X

  Logger(const Logger&) = delete;
  Logger& operator =(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator =(Logger&&) = delete;
private:
  Logger(const QDir& logs_dir);
  ~Logger();

  void log_internal(LogLevel level, std::string_view sv);
  QFile m_file{};
  QTextStream m_stream{};
  QDateTime m_creationTime{};
};
