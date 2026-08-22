#include "common.hpp"

QIcon loadIconFromSVG(const QString& path, bool customDarkTheme) {
  bool dark = customDarkTheme ? customDarkTheme : isDarkTheme();
  QColor tint = dark ? QColor("#e0e0e0") : QColor("#202020");

  QSvgRenderer renderer(path);
  QPixmap pixmap(22, 22);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  renderer.render(&painter);
  painter.end();

  QPixmap tinted(pixmap.size());
  tinted.fill(Qt::transparent);
  QPainter p2(&tinted);
  p2.drawPixmap(0, 0, pixmap);
  p2.setCompositionMode(QPainter::CompositionMode_SourceIn);
  p2.fillRect(tinted.rect(), tint);
  p2.end();

  return QIcon(tinted);
}
