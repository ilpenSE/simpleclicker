#pragma once

#include <QWidget>

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
  QPoint m_cursorPos;
  QRect m_lastCrosshairRect;
  QPixmap m_backgroundCache;
};
