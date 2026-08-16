#pragma once
#include <QStringLiteral>
#include <QString>
#include <format>

namespace emoji {
  const auto floppy_disk = QStringLiteral("\U0001F4BE");
  const auto mult_x = QStringLiteral("\u2715");
  const auto pencil = QStringLiteral("\u270E");
  const auto trash = QStringLiteral("\U0001F5D1");
}

template<>
struct std::formatter<QString> : std::formatter<std::string> {
  auto format(const QString& qstr, std::format_context& ctx) const {
    return std::formatter<std::string>::format(qstr.toStdString(), ctx);
  }
};
