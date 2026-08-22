#include "notificationbar.hpp"
#include "common.hpp"
#include <QHBoxLayout>

namespace {
constexpr int BAR_HEIGHT = 36;

QColor levelBaseColor(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Info:    return QColor("#2b6cb0");
  case NotificationLevel::Warning: return QColor("#b7791f");
  case NotificationLevel::Error:   return QColor("#b03030");
  default: return QColor(Qt::gray);
  }
}

QColor levelHighlightColor(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Info:    return QColor("#4299e1");
  case NotificationLevel::Warning: return QColor("#ecc94b");
  case NotificationLevel::Error:   return QColor("#f56565");
  default: return QColor(Qt::lightGray);
  }
}

} // namespace

NotificationBar::NotificationBar(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(12, 6, 8, 6);

  m_label = new QLabel(this);
  m_label->setStyleSheet("color: white; font-weight: 500;");
  m_label->setWordWrap(true);

  m_closeBtn = new QPushButton(this);
  m_closeBtn->setIcon(loadIconFromSVG(":/icons/cancel.svg"));
  m_closeBtn->setFlat(true);
  m_closeBtn->setCursor(Qt::PointingHandCursor);
  m_closeBtn->setStyleSheet("color: white; font-weight: bold; border: none;");
  m_closeBtn->setFixedSize(22, 22);

  layout->addWidget(m_label, 1);
  layout->addWidget(m_closeBtn);

  connect(m_closeBtn, &QPushButton::clicked, this, &NotificationBar::dismiss);

  m_autoHideTimer = new QTimer(this);
  m_autoHideTimer->setSingleShot(true);
  connect(m_autoHideTimer, &QTimer::timeout, this, &NotificationBar::dismiss);

  m_heightAnim = new QPropertyAnimation(this, "maximumHeight", this);
  m_heightAnim->setDuration(180);

  setMaximumHeight(0);
  setMinimumHeight(0);
}

void NotificationBar::show(const QString &message, NotificationLevel level, int timeoutMs) {
  m_queue.enqueue({message, level, timeoutMs});
  if (!m_showing) displayNext();
}

void NotificationBar::displayNext() {
  if (m_queue.empty()) {
    m_showing = false;
    stopFlash();
    m_heightAnim->stop();
    m_heightAnim->setStartValue(maximumHeight());
    m_heightAnim->setEndValue(0);
    m_heightAnim->start();
    return;
  }

  m_showing = true;
  auto item = m_queue.dequeue();
  m_label->setText(item.msg);

  QColor base = levelBaseColor(item.level);
  QColor highlight = levelHighlightColor(item.level);
  setBgColor(base);

  m_heightAnim->stop();
  m_heightAnim->setStartValue(maximumHeight());
  m_heightAnim->setEndValue(BAR_HEIGHT);
  m_heightAnim->start();

  switch (item.level) {
  case NotificationLevel::Warning: startFlash(base, highlight, 3); break;
  case NotificationLevel::Error: startFlash(base, highlight, -1); break;
  default: break;
  }

  m_autoHideTimer->stop();
  int timeout = item.timeout;
  if (timeout < 0 && item.level != NotificationLevel::Error) {
    timeout = (item.level == NotificationLevel::Info) ? 3500 : 5000;
  }
  if (timeout > 0) {
    m_autoHideTimer->start(timeout);
  }
}

void NotificationBar::startFlash(const QColor &base, const QColor &highlight, int loops) {
  stopFlash();
  m_flashAnim = new QPropertyAnimation(this, "bgColor", this);
  m_flashAnim->setDuration(500);
  m_flashAnim->setKeyValueAt(0.0, base);
  m_flashAnim->setKeyValueAt(0.5, highlight);
  m_flashAnim->setKeyValueAt(1.0, base);
  m_flashAnim->setLoopCount(loops); // -1 infinite
  m_flashAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationBar::stopFlash() {
  if (m_flashAnim) {
    m_flashAnim->stop();
  }
  m_flashAnim = nullptr;
}

void NotificationBar::dismiss() {
  m_autoHideTimer->stop();
  stopFlash();
  displayNext();
}

void NotificationBar::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.fillRect(rect(), m_bgColor);
}

void NotificationBar::setBgColor(const QColor &c) {
  m_bgColor = c;
  update();
}
