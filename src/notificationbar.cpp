#include "notificationbar.hpp"
#include <algorithm>
#include "common.hpp"
#include <QHBoxLayout>
#include "theme.hpp"
extern ThemeManager *thememan;

namespace {
constexpr int MIN_BAR_HEIGHT = 36;
constexpr int MAX_BAR_HEIGHT = 160;

int levelLoops(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Warning: return 2;
  case NotificationLevel::Error: return 3;
  case NotificationLevel::Success: return 1;
  default: return 0;
  }
}

int levelTimeout(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Warning: return 5000;
  case NotificationLevel::Error: return -1;
  default: return 3500;
  }
}

QColor levelBaseColor(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Info:    return thememan->color("notification-info-base");
  case NotificationLevel::Success: return thememan->color("notification-success-base");
  case NotificationLevel::Warning: return thememan->color("notification-warning-base");
  case NotificationLevel::Error:   return thememan->color("notification-error-base");
  default: return thememan->color("notification-default-base");
  }
}

QColor levelHighlightColor(NotificationLevel level) {
  switch (level) {
  case NotificationLevel::Info:    return thememan->color("notification-info-highlight");
  case NotificationLevel::Success: return thememan->color("notification-success-highlight");
  case NotificationLevel::Warning: return thememan->color("notification-warning-highlight");
  case NotificationLevel::Error:   return thememan->color("notification-error-highlight");
  default: return thememan->color("notification-default-highlight");
  }
}

constexpr int LAYOUT_LEFT_MARGIN = 12;
constexpr int LAYOUT_RIGHT_MARGIN = 8;
constexpr int LAYOUT_TOP_MARGIN = 6;
constexpr int LAYOUT_BOT_MARGIN = 6;
constexpr int LAYOUT_SPACING = 6;

} // namespace

NotificationBar::NotificationBar(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(LAYOUT_LEFT_MARGIN, LAYOUT_TOP_MARGIN, LAYOUT_RIGHT_MARGIN, LAYOUT_BOT_MARGIN);
  layout->setSpacing(LAYOUT_SPACING);

  m_label = new QLabel(this);
  m_label->setWordWrap(true);

  m_closeBtn = new QPushButton(this);
  makeDynamicIconButton(m_closeBtn, "cancel.svg");
  m_closeBtn->setFlat(true);
  m_closeBtn->setCursor(Qt::PointingHandCursor);
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
  dismiss(); // overwrite current showing one
  m_showing = true;

  m_label->setText(message);
  const int targetHeight = computeRequiredHeight(message);

  const QColor base = levelBaseColor(level);
  const QColor highlight = levelHighlightColor(level);
  setBgColor(base);

  m_heightAnim->stop();
  m_heightAnim->setStartValue(maximumHeight());
  m_heightAnim->setEndValue(targetHeight);
  m_heightAnim->start();

  int loops = levelLoops(level);
  if (loops != 0) startFlash(base, highlight, loops);

  int timeout = timeoutMs;
  if (timeout < 0 && level != NotificationLevel::Error) {
    timeout = levelTimeout(level);
  }
  if (timeout > 0) {
    m_autoHideTimer->start(timeout);
  }
}

void NotificationBar::dismiss() {
  m_autoHideTimer->stop();
  stopFlash();
  m_heightAnim->stop();
  m_heightAnim->setStartValue(maximumHeight());
  m_heightAnim->setEndValue(0);
  m_heightAnim->start();
  emit dismissed();
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

void NotificationBar::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.fillRect(rect(), m_bgColor);
}

void NotificationBar::setBgColor(const QColor &c) {
  m_bgColor = c;
  update();
}

int NotificationBar::computeRequiredHeight(const QString &text) const {
  const int margins = LAYOUT_LEFT_MARGIN + LAYOUT_RIGHT_MARGIN;
  const int closeBtnSpace = m_closeBtn->width() + LAYOUT_SPACING;
  int availableWidth = width() - margins - closeBtnSpace;
  if (availableWidth <= 0) availableWidth = 300;

  QFontMetrics fm(m_label->font());
  QRect bounds = fm.boundingRect(QRect(0, 0, availableWidth, 0),
                                 Qt::TextWordWrap, text);

  const int verticalPadding = LAYOUT_TOP_MARGIN + LAYOUT_BOT_MARGIN;
  int required = bounds.height() + verticalPadding;

  return std::clamp(required, static_cast<int>(MIN_BAR_HEIGHT), static_cast<int>(MAX_BAR_HEIGHT));
}
