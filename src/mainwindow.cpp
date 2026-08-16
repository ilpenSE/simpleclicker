#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.hpp"
#include "logger.hpp"
#include "presets.hpp"
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
  for (auto [key, value] : presets->map.asKeyValueRange()) {
    addPresetItem(*ui->presetsList, key);
  }

  // set current (active) preset head item in the list for now
  // because we dont have any settings right now
  auto *currentItem = ui->presetsList->item(0);
  if (currentItem) {
    currentPresetWidget = qobject_cast<PresetItemWidget*>(ui->presetsList->itemWidget(currentItem));
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

void MainWindow::addPresetItem(QListWidget& list, const QString& presetName) {
  PresetItemWidget *itemWidget = new PresetItemWidget(presetName, &list);
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(itemWidget->sizeHint());
  list.addItem(item);
  list.setItemWidget(item, itemWidget);

  // Delete an item from preset
  connect(itemWidget, &PresetItemWidget::deleteRequested, this, [&list, item, itemWidget]() {
    lg->info("removing preset: '{}'", itemWidget->presetName());
    presets->map.remove(itemWidget->presetName());
    delete list.takeItem(list.row(item));
  });

  // Rename the preset name
  connect(itemWidget, &PresetItemWidget::renameRequested, this, [itemWidget](const QString &newName) {
    lg->info("renaming preset: '{}' -> '{}'", itemWidget->presetName(), newName);
    struct PresetConfig cfg = presets->map.take(itemWidget->presetName());
    presets->map.insert(newName, std::move(cfg));
  });

  // Save preset
  connect(itemWidget, &PresetItemWidget::saveRequested, this, [itemWidget]() {
    lg->info("saving preset: '{}'", itemWidget->presetName());
    itemWidget->markSaved();
  });
}

MainWindow::~MainWindow()
{
  delete ui;
}
