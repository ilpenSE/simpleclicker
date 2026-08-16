#include "logger.hpp"
#include <QDateTime>
#include <QIODeviceBase>

Logger::Logger(const QDir& logs_dir) : m_file(logs_dir.filePath("latest.log")), m_stream(&m_file), m_creationTime(QDateTime::currentDateTime())
{}

Logger::~Logger() {
  m_file.rename(m_creationTime.toString("dd-MM-yyyy hh:mm:ss"));
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
  QString final_message = QString("%1 [%2] %3")
    .arg(time_qstr, to_cstr(level), QString::fromUtf8(sv.data(), sv.size()));

  m_stream << final_message;
  qDebug().noquote() << final_message;
}
