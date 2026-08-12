#include "logger.hpp"
#include <QDateTime>
#include <cstdio>

bool Logger::init(const char *file_path) {
  if (!file_path) return false;
  m_file_path = file_path;
  m_file = fopen(file_path, "w");
  if (!m_file) return false;
  return true;
}

Logger::~Logger() {
  fclose(m_file);
}

void Logger::log_internal(LogLevel level, std::string_view sv) {
  if (!m_file) return;
  QString time_qstr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
  const char *time_str = time_qstr.toUtf8().constData();

  char buffer[2048];
  int n = snprintf(buffer, sizeof(buffer), "%s [%s] %.*s\n",
                   time_str, to_cstr(level), (int)sv.size(), sv.data());
  if (n < 0) return;

  fprintf(m_file, "%.*s", n, buffer);
  printf("%.*s", n, buffer);
}
