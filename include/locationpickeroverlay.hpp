#pragma once

#include <QWidget>
#include <QPoint>

class LocationPickerOverlay : public QWidget {
  Q_OBJECT
public:
  explicit LocationPickerOverlay(QWidget *parent = nullptr);

signals:
  void locationPicked(QPoint pos);
  void cancelled();

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void showEvent(QShowEvent *event) override;

private:
  void updateCursor(const QPoint &globalPos);

  QPoint m_cursorPos;
  QRect m_lastCrosshairRect;
  QPixmap m_backgroundCache;
  QRect m_infoRect;
  QString m_infoString;
};
