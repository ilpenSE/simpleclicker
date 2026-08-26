#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "notificationbar.hpp"
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
  explicit MainWindow(QString initPreset, QWidget *parent = nullptr);
  ~MainWindow() override;

public slots:
  void applyTheme(Theme newTheme);

private:
  void showLocationPicker();
  MouseButton getMouseButton() const;
  int getIntervalMs() const;

  void applySettings();
  void applyPreset(const PresetConfig& config);
  QListWidgetItem* addPresetItem(QListWidget& list, const QString& presetName, const PresetConfig& config);
  void setActivePreset(QListWidgetItem *item);

  void blockPresetConfigUi() { _changePresetConfigUi(true); }
  void unblockPresetConfigUi() { _changePresetConfigUi(false); }
  void _changePresetConfigUi(bool is_locked);

  void retranslateUi();

  void startClicking();
  void stopClicking();

  QString generateUniquePresetName() const;
  Ui::MainWindow *ui;
  NotificationBar *m_notificationBar;
  QString m_currentPreset;
};
#endif // MAINWINDOW_H
