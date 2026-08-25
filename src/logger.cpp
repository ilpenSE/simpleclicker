#include "logger.hpp"
#include <QDateTime>
#include <QIODeviceBase>
#include <iostream>

namespace {
constexpr const char *levelColor(LogLevel level) {
  switch (level) {
  case LogLevel::INFO: return "\e[0;32m";
  case LogLevel::ERROR: return "\e[0;31m";
  case LogLevel::WARNING: return "\e[0;33m";
  default: return "\e[0m";
  }
}
} // namespace

Logger::Logger(const QDir& logs_dir)
  : m_file(logs_dir.filePath("latest.log")),
    m_stream(&m_file),
    m_creationTime(QDateTime::currentDateTime())
{}

Logger::~Logger() {
  const QString archived = m_file.fileName().section('/', 0, -2) + "/" +
                           m_creationTime.toString("dd-MM-yyyy-hh-mm-ss") + ".log";
  m_file.copy(m_file.fileName(), archived);
  m_file.close();
}

void Logger::log_internal(LogLevel level, std::string_view sv) {
  if (!m_file.isOpen()) {
    if (!m_file.open(QIODeviceBase::WriteOnly | QIODevice::Text)) {
#ifndef NDEBUG
      fprintf(stderr, "ERROR: Couldn't open log file\n");
#endif
      return;
    }
  }

  QString time_qstr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
  m_stream << time_qstr << " [" << to_cstr(level) << "] " << QString::fromUtf8(sv.data(), sv.size()) << Qt::endl;

#ifndef NDEBUG
  auto rst = "\e[0m";
  auto cyan = "\e[0;36m";
  auto level_color = levelColor(level);

  std::cout << cyan << time_qstr.toUtf8().constData() << rst << " [" << level_color << to_cstr(level) << rst << "] " << sv << std::endl;
#endif
}
