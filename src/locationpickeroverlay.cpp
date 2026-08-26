#include "locationpickeroverlay.hpp"
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QPainter>

LocationPickerOverlay::LocationPickerOverlay(QWidget *parent)
    : QWidget(parent)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);
  setCursor(Qt::BlankCursor);
  setMouseTracking(true);

  QRect fullGeom;
  for (auto *screen : QGuiApplication::screens())
    fullGeom = fullGeom.united(screen->geometry());
  setGeometry(fullGeom);
}

void LocationPickerOverlay::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  activateWindow();
  raise();
  grabMouse();
  grabKeyboard();

  m_backgroundCache = QPixmap(size());
  m_backgroundCache.fill(Qt::transparent);
  QPainter bgPainter(&m_backgroundCache);
  bgPainter.fillRect(rect(), QColor(0, 0, 0, 120));
}

QRect crosshairBounds(const QPoint &localPos) {
  // crosshair (30x30) + label
  return QRect(localPos.x() - 60, localPos.y() - 30, 180, 60);
}

void LocationPickerOverlay::mouseMoveEvent(QMouseEvent *event) {
  m_cursorPos = event->globalPosition().toPoint();
  QPoint local = m_cursorPos - geometry().topLeft();

  QRect newRect = crosshairBounds(local);
  QRect dirty = m_lastCrosshairRect.united(newRect);
  m_lastCrosshairRect = newRect;

  update(dirty);
}

void LocationPickerOverlay::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QPoint pos = event->globalPosition().toPoint();
    releaseMouse();
    releaseKeyboard();
    emit locationPicked(pos);
    close();
  }
}

void LocationPickerOverlay::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    releaseMouse();
    releaseKeyboard();
    emit cancelled();
    close();
  }
}

void LocationPickerOverlay::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.drawPixmap(event->rect(), m_backgroundCache, event->rect());
  QPoint local = m_cursorPos - geometry().topLeft();

  // Crosshair
  p.setPen(QPen(Qt::white, 1));
  p.drawLine(local.x() - 15, local.y(), local.x() + 15, local.y());
  p.drawLine(local.x(), local.y() - 15, local.x(), local.y() + 15);

  // Coordinate label
  QString label = QString("%1, %2").arg(m_cursorPos.x()).arg(m_cursorPos.y());
  p.setPen(Qt::white);
  p.drawText(local + QPoint(20, -10), label);
}
