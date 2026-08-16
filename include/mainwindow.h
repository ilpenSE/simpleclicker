#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "presets.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private:
  MouseButton getMouseButton() const;
  size_t getIntervalMs() const;

  void applySettings();
  void applyPreset(const PresetConfig& config);
  void addPresetItem(QListWidget& list, const QString& presetName, const PresetConfig& config);
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
