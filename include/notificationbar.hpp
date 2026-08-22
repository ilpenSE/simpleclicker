#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include <QQueue>
#include <QPointer>

enum class NotificationLevel {
  Info, Warning, Error, Count,
};

class NotificationBar : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)

public:
  explicit NotificationBar(QWidget *parent = nullptr);

  void show(const QString &message, NotificationLevel level, int timeoutMs = -1);

  void info(const QString &message, int timeoutMs = -1) { return show(message, NotificationLevel::Info, timeoutMs); }
  void error(const QString &message, int timeoutMs = -1) { return show(message, NotificationLevel::Error, timeoutMs); }
  void warning(const QString &message, int timeoutMs = -1) { return show(message, NotificationLevel::Warning, timeoutMs); }

  QColor bgColor() const { return m_bgColor; }
  void setBgColor(const QColor &c);

protected:
  void paintEvent(QPaintEvent *event) override;
  QSize sizeHint() const override {
    return QSize(width(), maximumHeight());
  }

private:
  struct QueueItem { QString msg; NotificationLevel level; int timeout; };

  void displayNext();
  void startFlash(const QColor &base, const QColor &highlight, int loops);
  void stopFlash();
  void dismiss();

  QLabel *m_label;
  QPushButton *m_closeBtn;
  QPointer<QPropertyAnimation> m_flashAnim = nullptr;
  QPropertyAnimation *m_heightAnim;
  QTimer *m_autoHideTimer;
  QColor m_bgColor{Qt::transparent};
  QQueue<QueueItem> m_queue;
  bool m_showing = false;
};
