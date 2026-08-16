#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.hpp"
#include "logger.hpp"
#include "presetitemwidget.hpp"

extern Logger *lg;
extern PresetManager *presets;

static PresetItemWidget *currentPresetWidget;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  lg->info("Hello from MainWindow!");

  // Add presets to the list
  for (const auto& [name, cfg] : presets->keyValueRange()) {
    addPresetItem(*ui->presetsList, name, cfg);
  }

  // set current (active) preset head item in the list for now
  // because we dont have any settings right now
  auto *currentItem = ui->presetsList->item(0);
  if (currentItem) {
    currentPresetWidget = qobject_cast<PresetItemWidget*>(ui->presetsList->itemWidget(currentItem));
    applyPreset(currentPresetWidget->config);
  } else currentPresetWidget = nullptr;

  // Connect to preset config changes on UI
  auto markUnsaved = [](){ if (currentPresetWidget) currentPresetWidget->markUnsaved(); };
  for (auto it : { ui->hoursEdit, ui->minutesEdit, ui->secondsEdit, ui->millisEdit,
                   ui->repeatEdit, ui->xEdit, ui->yEdit }) {
    connect(it, &QSpinBox::valueChanged, this, markUnsaved);
  }
  for (auto it : { ui->repeatCountRadio, ui->currentLocationRadio }) {
    connect(it, &QRadioButton::toggled, this, markUnsaved);
  }
  connect(ui->mouseBtnCombo, &QComboBox::currentIndexChanged, this, markUnsaved);
}

void MainWindow::addPresetItem(QListWidget& list, const QString& presetName, const PresetConfig& config) {
  PresetItemWidget *itemWidget = new PresetItemWidget(presetName, config, &list);
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(itemWidget->sizeHint());
  list.addItem(item);
  list.setItemWidget(item, itemWidget);

  // Delete an item from preset
  connect(itemWidget, &PresetItemWidget::deleteRequested, this, [&list, item, itemWidget]() {
    lg->info("removing preset: '{}'", itemWidget->presetName());
    presets->take(itemWidget->presetName());
    delete list.takeItem(list.row(item));
  });

  // Rename the preset name
  connect(itemWidget, &PresetItemWidget::renameRequested, this, [itemWidget](const QString &newName) {
    lg->info("renaming preset: '{}' -> '{}'", itemWidget->presetName(), newName);
    auto cfg = presets->take(itemWidget->presetName());
    presets->insert(newName, cfg);
  });

  // Save preset
  connect(itemWidget, &PresetItemWidget::saveRequested, this, [this, itemWidget]() {
    lg->info("saving preset: '{}'", itemWidget->presetName());
    struct PresetConfig cfg{
      .interval=getIntervalMs(),
      .loc=getLocation(),
      .mouse=getMouseButton(),
      .repeat=getRepeat(),
      .current_loc=ui->currentLocationRadio->isChecked(),
      .repeat_forever=ui->repeatUntilStoppedRadio->isChecked(),
    };

    presets->set(itemWidget->presetName(), cfg);
    itemWidget->config = cfg;
    itemWidget->markSaved();
  });
}

void MainWindow::applyPreset(const PresetConfig& config) {
  size_t interval = config.interval;
  ui->hoursEdit->setValue(interval / 60 / 60 / 1000);
  ui->minutesEdit->setValue(interval / 60 / 1000);
  ui->secondsEdit->setValue(interval / 1000);
  ui->millisEdit->setValue(interval % 1000);

  ui->mouseBtnCombo->setCurrentIndex(static_cast<int>(config.mouse));
  ui->repeatEdit->setValue(config.repeat);
  ui->repeatUntilStoppedRadio->setChecked(config.repeat_forever);
  ui->repeatCountRadio->setChecked(!config.repeat_forever);
  ui->currentLocationRadio->setChecked(config.current_loc);
  ui->pickLocationRadio->setChecked(!config.current_loc);
  ui->xEdit->setValue(config.loc.x);
  ui->yEdit->setValue(config.loc.y);
}

int MainWindow::getRepeat() const {
  if (ui->repeatUntilStoppedRadio->isChecked()) return default_repeat;
  return ui->repeatEdit->value();
}

Location MainWindow::getLocation() const {
  if (ui->currentLocationRadio->isChecked()) return default_loc;
  return {ui->xEdit->value(), ui->yEdit->value()};
}

MouseButton MainWindow::getMouseButton() const {
  int idx = ui->mouseBtnCombo->currentIndex();
  if (idx == -1) return default_mouse_button;
  return static_cast<MouseButton>(idx);
}

size_t MainWindow::getIntervalMs() const {
  size_t res = 0;
  res += ui->hoursEdit->value() * 60 * 60 * 1000;
  res += ui->minutesEdit->value() * 60 * 1000;
  res += ui->secondsEdit->value() * 1000;
  res += ui->millisEdit->value();
  return res;
}

MainWindow::~MainWindow()
{
  presets->save();
  delete ui;
}
