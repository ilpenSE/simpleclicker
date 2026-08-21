#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.hpp"
#include "logger.hpp"
#include "settings.hpp"
#include "presetitemwidget.hpp"
#include <QShortcut>

extern Logger *lg;
extern PresetManager *presetsman;
extern SettingsManager *settingsman;

static PresetItemWidget *currentPresetWidget = nullptr;
constexpr auto SAVE_CHANGES_KEYBIND = "Ctrl+S";
constexpr auto ABORT_CHANGES_KEYBIND = "Ctrl+K";
static bool presetChangeEventLock = false;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  lg->info("Hello from MainWindow!");
  ui->helpLabel->setVisible(false);

  // Add presets to the list
  for (const auto& [name, cfg] : presetsman->presets.asKeyValueRange()) {
    addPresetItem(*ui->presetsList, name, cfg);
  }

  applySettings();

  // Bind Save keybind
  auto *saveShortcut = new QShortcut(QKeySequence(SAVE_CHANGES_KEYBIND), this);
  connect(saveShortcut, &QShortcut::activated, this, [this]() {
    if (currentPresetWidget) emit currentPresetWidget->saveRequested();
  });

  // Bind Cancel keybind
  auto *cancelShortcut = new QShortcut(QKeySequence(ABORT_CHANGES_KEYBIND), this);
  connect(cancelShortcut, &QShortcut::activated, this, [this]() {
    if (currentPresetWidget) emit currentPresetWidget->cancelRequested();
  });

  // Connect to preset config changes on UI
  auto markUnsaved = [](){ if (currentPresetWidget && !presetChangeEventLock) currentPresetWidget->markUnsaved(); };
  for (auto it : { ui->hoursEdit, ui->minutesEdit, ui->secondsEdit, ui->millisEdit,
                   ui->repeatEdit, ui->xEdit, ui->yEdit }) {
    connect(it, &QSpinBox::valueChanged, this, markUnsaved);
  }
  for (auto it : { ui->repeatCountRadio, ui->currentLocationRadio }) {
    connect(it, &QRadioButton::toggled, this, markUnsaved);
  }
  connect(ui->mouseBtnCombo, &QComboBox::currentIndexChanged, this, markUnsaved);

  // Set up addPreset button
  ui->addPresetBtn->setIcon(loadIconFromSVG(":/icons/add.svg"));
  connect(ui->addPresetBtn, &QPushButton::clicked, this, [this](){
      QString newPresetName = QString("New Preset %1").arg(presetsman->presets.count() + 1);
      QListWidgetItem *newItem = addPresetItem(*ui->presetsList, newPresetName, {});
      setActivePreset(newItem);
      PresetItemWidget *newItemWidget = qobject_cast<PresetItemWidget*>(
          ui->presetsList->itemWidget(newItem));
      newItemWidget->enterEditMode();
      presetsman->presets[newPresetName] = {};
  });

  // If there's no current preset, block the preset config UI
  if (!currentPresetWidget) {
    blockPresetConfigUi();
  }
}

void MainWindow::setActivePreset(QListWidgetItem *item) {
  // TODO: introduce better visuals for active elements, for now we have focus
  if (!item) {
    applyPreset({});
    currentPresetWidget = nullptr;
    lg->info("Setting active preset to null");
    blockPresetConfigUi();
    return;
  }

  unblockPresetConfigUi();

  auto itemWidget = qobject_cast<PresetItemWidget*>(ui->presetsList->itemWidget(item));
  applyPreset(itemWidget->config);
  ui->presetsList->setCurrentItem(item);
  itemWidget->setActive(true);
  currentPresetWidget = itemWidget;
  lg->info("Setting active preset to '{}'", itemWidget->presetName());
}

QListWidgetItem* MainWindow::addPresetItem(QListWidget& list, const QString& presetName, const PresetConfig& config) {
  PresetItemWidget *itemWidget = new PresetItemWidget(presetName, config, &list);
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(itemWidget->sizeHint());
  list.addItem(item);
  list.setItemWidget(item, itemWidget);

  static auto cur_preset = settingsman->get<QString>("currentPreset");
  if (!currentPresetWidget && presetName == cur_preset) {
    lg->info("Found current preset: '{}'", cur_preset);
    setActivePreset(item);
  }

  // Delete an item from preset
  connect(itemWidget, &PresetItemWidget::deleteRequested, this, [this, &list, item, itemWidget]() {
    lg->info("Removing preset: '{}'", itemWidget->presetName());
    presetsman->presets.remove(itemWidget->presetName());
    int idx = list.row(item);
    if (currentPresetWidget == itemWidget) {
      // if current's gonna be deleted, set active preset to something else
      setActivePreset(idx <= 0 ? nullptr : list.item(idx - 1));
    }

    list.removeItemWidget(item);
    delete list.takeItem(idx);
  });

  // Rename the preset name
  connect(itemWidget, &PresetItemWidget::renameRequested, this, [itemWidget](const QString &newName) {
    lg->info("Renaming preset: '{}' -> '{}'", itemWidget->presetName(), newName);
    auto cfg = presetsman->presets.take(itemWidget->presetName());
    presetsman->presets.insert(newName, cfg);
  });

  // Abort changes, cancel requested
  connect(itemWidget, &PresetItemWidget::cancelRequested, this, [this, itemWidget]() {
    if (currentPresetWidget) applyPreset(currentPresetWidget->config);
    itemWidget->markSaved();
  });

  // Save preset
  connect(itemWidget, &PresetItemWidget::saveRequested, this, [this, itemWidget]() {
    auto pname = itemWidget->presetName();
    lg->info("Saving preset: '{}'", pname);
     PresetConfig newConfig = {
      .location = Location{ui->xEdit->value(), ui->yEdit->value()},
      .interval = getIntervalMs(),
      .repeat = ui->repeatEdit->value(),
      .mouseButton = getMouseButton(),
      .repeatUntilStopped = ui->repeatUntilStoppedRadio->isChecked(),
      .currentLocation = ui->currentLocationRadio->isChecked(),
    };
    presetsman->presets[pname] = newConfig;
    itemWidget->config = newConfig;
    itemWidget->markSaved();
  });

  // Double clicked, set current preset to this
  connect(itemWidget, &PresetItemWidget::doubleClicked, this, [this, item](){
    setActivePreset(item);
  });

  return item;
}

void MainWindow::_changePresetConfigUi(bool is_locked) {
  for (auto it : {ui->clickOptionsBox, ui->clickIntervalBox, ui->repeatBox, ui->positionBox}) {
    it->setEnabled(!is_locked);
  }

  ui->startButton->setEnabled(!is_locked);
  ui->stopButton->setEnabled(!is_locked);

  ui->helpLabel->setVisible(is_locked);
}

void MainWindow::applySettings() {
  auto kbd = settingsman->get<QString>("keybind");
  ui->startButton->setText(QString("START (%1)").arg(kbd));
  ui->stopButton->setText(QString("STOP (%1)").arg(kbd));
}

void MainWindow::applyPreset(const PresetConfig& config) {
  presetChangeEventLock = true;
  size_t interval = config.interval;
  ui->hoursEdit->setValue(interval / 60 / 60 / 1000);
  ui->minutesEdit->setValue(interval / 60 / 1000);
  ui->secondsEdit->setValue(interval / 1000);
  ui->millisEdit->setValue(interval % 1000);

  ui->mouseBtnCombo->setCurrentIndex(static_cast<int>(config.mouseButton));
  ui->repeatEdit->setValue(config.repeat);
  ui->repeatUntilStoppedRadio->setChecked(config.repeatUntilStopped);
  ui->repeatCountRadio->setChecked(!config.repeatUntilStopped);
  ui->currentLocationRadio->setChecked(config.currentLocation);
  ui->pickLocationRadio->setChecked(!config.currentLocation);
  ui->xEdit->setValue(config.location.x);
  ui->yEdit->setValue(config.location.y);
  presetChangeEventLock = false;
}

MouseButton MainWindow::getMouseButton() const {
  int idx = ui->mouseBtnCombo->currentIndex();
  if (idx == -1) return {};
  return static_cast<MouseButton>(idx);
}

int MainWindow::getIntervalMs() const {
  int res = 0;
  res += ui->hoursEdit->value() * 60 * 60 * 1000;
  res += ui->minutesEdit->value() * 60 * 1000;
  res += ui->secondsEdit->value() * 1000;
  res += ui->millisEdit->value();
  return res;
}

MainWindow::~MainWindow()
{
  settingsman->set("currentPreset", currentPresetWidget ? currentPresetWidget->presetName() : "");

  settingsman->save();
  presetsman->save();
  delete ui;
}
