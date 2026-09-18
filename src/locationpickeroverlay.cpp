#include "locationpickeroverlay.hpp"
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QPainter>

namespace {
QRect crosshairBounds(const QPoint &localPos) {
  // crosshair (30x30) + label
  return QRect(localPos.x() - 60, localPos.y() - 30, 180, 60);
}
constexpr auto CROSSHAIR_SIZE = 15;
} // namespace

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

  m_infoString = tr("Click anywhere to pick a location, press ESC to cancel picking.");
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

  static const QFontMetrics fm(font());
  static auto box_padding_size = QSize(20, 20);
  const QSize box_size = fm.size(0, m_infoString) + box_padding_size;

  QScreen *screen = qApp->screenAt(QCursor::pos());
  if (!screen) screen = qApp->primaryScreen();
  m_infoRect = QRect(QPoint(), box_size);
  m_infoRect.moveCenter(screen->geometry().center() - geometry().topLeft());

  updateCursor(QCursor::pos());
}

void LocationPickerOverlay::updateCursor(const QPoint &globalPos) {
  m_cursorPos = globalPos;
  const QPoint local = m_cursorPos - geometry().topLeft();
  const QRect newRect = crosshairBounds(local);

  // Update info rect if it intersects with cross rect
  auto dirty = m_lastCrosshairRect.united(newRect);
  bool wasCovered = m_lastCrosshairRect.intersects(m_infoRect);
  bool isCovered = newRect.intersects(m_infoRect);
  if (isCovered != wasCovered) {
    dirty = dirty.united(m_infoRect);
  }

  m_lastCrosshairRect = newRect;
  update(dirty);
}

void LocationPickerOverlay::mouseMoveEvent(QMouseEvent *event) {
  updateCursor(event->globalPosition().toPoint());
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
  const QPoint local = m_cursorPos - geometry().topLeft();

  // Draw if it needs to be updated (event->rect() = dirty)
  if (event->rect().intersects(m_infoRect)) {
    bool covered = crosshairBounds(local).intersects(m_infoRect);
    p.setOpacity(covered ? 0.25 : 1.0);

    // Info text's rectangle
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 20, 20, 210));
    p.drawRoundedRect(m_infoRect, 8, 8);

    // Info Text
    p.setPen(Qt::white);
    p.drawText(m_infoRect, Qt::AlignCenter, m_infoString);

    p.setBrush(Qt::NoBrush);
    p.setOpacity(1.0);
  }

  // Crosshair
  p.setPen(QPen(Qt::white, 1));
  p.drawLine(local.x() - CROSSHAIR_SIZE, local.y(), local.x() + CROSSHAIR_SIZE, local.y());
  p.drawLine(local.x(), local.y() - CROSSHAIR_SIZE, local.x(), local.y() + CROSSHAIR_SIZE);

  // Coordinate label
  QString label = QString("%1, %2").arg(m_cursorPos.x()).arg(m_cursorPos.y());
  p.setPen(Qt::white);
  p.drawText(local + QPoint(20, -10), label);
}
