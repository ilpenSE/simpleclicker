#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include <QQueue>
#include <QPointer>

enum class NotificationLevel {
  Info, Success, Warning, Error, Count,
};

class NotificationBar : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)

public:
  explicit NotificationBar(QWidget *parent = nullptr);

  void show(const QString &message, NotificationLevel level, int timeoutMs = -1);
  void dismiss();

  void info(const QString &message, int timeoutMs = -1) {
    return show(message, NotificationLevel::Info, timeoutMs);
  }

  void success(const QString &message, int timeoutMs = -1) {
    return show(message, NotificationLevel::Success, timeoutMs);
  }

  void error(const QString &message, int timeoutMs = -1) {
    return show(message, NotificationLevel::Error, timeoutMs);
  }

  void warning(const QString &message, int timeoutMs = -1) {
    return show(message, NotificationLevel::Warning, timeoutMs);
  }

  QColor bgColor() const { return m_bgColor; }
  void setBgColor(const QColor &c);

protected:
  void paintEvent(QPaintEvent *event) override;
  QSize sizeHint() const override {
    return QSize(width(), maximumHeight());
  }

signals:
  void dismissed();

private:
  int computeRequiredHeight(const QString &text) const;
  void displayNext();
  void startFlash(const QColor &base, const QColor &highlight, int loops);
  void stopFlash();

  QLabel *m_label;
  QPushButton *m_closeBtn;
  QPointer<QPropertyAnimation> m_flashAnim = nullptr;
  QPropertyAnimation *m_heightAnim;
  QTimer *m_autoHideTimer;
  QColor m_bgColor{Qt::transparent};
  bool m_showing = false;
};
