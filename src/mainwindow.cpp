#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.hpp"
#include "logger.hpp"
#include "settings.hpp"
#include "theme.hpp"
#include "presetitemwidget.hpp"
#include <QShortcut>

extern Logger *lg;
extern PresetManager *presetsman;
extern SettingsManager *settingsman;
extern ThemeManager *thememan;

static PresetItemWidget *currentPresetWidget = nullptr;
constexpr auto SAVE_CHANGES_KEYBIND = "Ctrl+S";
constexpr auto ABORT_CHANGES_KEYBIND = "Ctrl+K";
static bool presetChangeEventLock = false;
static bool uiConstructed = false;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  m_notificationBar = new NotificationBar(this);
  qobject_cast<QVBoxLayout *>(centralWidget()->layout())->insertWidget(0, m_notificationBar);

  lg->info("MainWindow initialized");

  // Set up theme manager
  connect(thememan, &ThemeManager::themeChanged, this, &MainWindow::applyTheme);
  applyTheme(thememan->theme());

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
  auto markUnsaved = [](){ if (currentPresetWidget && !presetChangeEventLock && uiConstructed) currentPresetWidget->markUnsaved(); };
  for (auto it : { ui->hoursEdit, ui->minutesEdit, ui->secondsEdit, ui->millisEdit,
                   ui->repeatEdit, ui->xEdit, ui->yEdit }) {
    connect(it, &QSpinBox::valueChanged, this, markUnsaved);
  }
  for (auto it : { ui->repeatCountRadio, ui->currentLocationRadio }) {
    connect(it, &QRadioButton::toggled, this, markUnsaved);
  }
  connect(ui->mouseBtnCombo, &QComboBox::currentIndexChanged, this, markUnsaved);

  // Set up addPreset button
  makeDynamicIconButton(ui->addPresetBtn, "add.svg");
  connect(ui->addPresetBtn, &QPushButton::clicked, this, [this](){
    QString newPresetName = generateUniquePresetName();
    QListWidgetItem *newItem = addPresetItem(*ui->presetsList, newPresetName, {});
    setActivePreset(newItem);
    PresetItemWidget *newItemWidget = qobject_cast<PresetItemWidget*>(ui->presetsList->itemWidget(newItem));
    newItemWidget->enterEditMode();
    presetsman->presets[newPresetName] = {};
    m_notificationBar->success(QString("Created new preset '%1'").arg(newPresetName));
  });

  // If there's no current preset, block the preset config UI
  if (!currentPresetWidget) {
    blockPresetConfigUi();

    if (presetsman->presets.size() == 0) {
      m_notificationBar->info("Please create a new preset using (+) button.", 5 * 60 * 1000);
    } else {
      m_notificationBar->info("Please select a preset from left panel.", 5 * 60 * 1000);
    }
  }

  // For test purposes
  connect(ui->settingsBtn, &QPushButton::clicked, this, []() {
    Theme now = thememan->theme();
    thememan->setTheme(now == Theme::Light ? Theme::Dark : Theme::Light);
  });

  uiConstructed = true;
}

void MainWindow::setActivePreset(QListWidgetItem *item) {
  if (!item) {
    applyPreset({});
    if (currentPresetWidget) currentPresetWidget->setActive(false);
    currentPresetWidget = nullptr;
    lg->info("Setting active preset to null");
    blockPresetConfigUi();
    return;
  }

  unblockPresetConfigUi();

  auto itemWidget = qobject_cast<PresetItemWidget*>(ui->presetsList->itemWidget(item));
  applyPreset(itemWidget->config);

  if (currentPresetWidget && currentPresetWidget != itemWidget) {
    currentPresetWidget->setActive(false);
  }

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
    m_notificationBar->success(QString("Removed preset '%1'.").arg(itemWidget->presetName()));

    list.removeItemWidget(item);
    delete list.takeItem(idx);
  });

  // Rename the preset name
  connect(itemWidget, &PresetItemWidget::renameRequested, this, [itemWidget, this](const QString &newName) {
    lg->info("Renaming preset: '{}' -> '{}'", itemWidget->presetName(), newName);
    if (presetsman->presets.contains(newName)) {
      m_notificationBar->warning(QString("Preset '%1' already exists, rename aborted").arg(newName));
      lg->warning("Preset '{}' already exists, rename aborted", newName);
      return;
    }

    auto cfg = presetsman->presets.take(itemWidget->presetName());
    presetsman->presets.insert(newName, cfg);
    m_notificationBar->success(QString("Renamed preset '%1' to '%2'").arg(itemWidget->presetName(), newName));
    itemWidget->setPresetName(newName);
  });

  // Abort changes, cancel requested
  connect(itemWidget, &PresetItemWidget::cancelRequested, this, [this, itemWidget]() {
    if (currentPresetWidget) applyPreset(currentPresetWidget->config);
    itemWidget->markSaved();
    m_notificationBar->info("Changes aborted.");
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
    m_notificationBar->success(QString("Saved preset '%1'.").arg(pname));
  });

  // Double clicked, set current preset to this
  connect(itemWidget, &PresetItemWidget::doubleClicked, this, [this, item, itemWidget]() {
    setActivePreset(item);
    m_notificationBar->success(QString("Set active preset to %1").arg(itemWidget->presetName()));
  });

  return item;
}

void MainWindow::_changePresetConfigUi(bool is_locked) {
  for (auto it : {ui->clickOptionsBox, ui->clickIntervalBox, ui->repeatBox, ui->positionBox}) {
    it->setEnabled(!is_locked);
  }

  ui->startButton->setEnabled(!is_locked);
  ui->stopButton->setEnabled(!is_locked);
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

QString MainWindow::generateUniquePresetName() const {
  int n = presetsman->presets.count() + 1;
  QString candidate;
  do {
    candidate = QString("New Preset %1").arg(n++);
  } while (presetsman->presets.contains(candidate));
  return candidate;
}

void MainWindow::applyTheme(Theme newTheme) {
  lg->info("Applied theme: '{}'", to_cstr(newTheme));
}

MainWindow::~MainWindow()
{
  settingsman->set("currentPreset", currentPresetWidget ? currentPresetWidget->presetName() : "");

  settingsman->save();
  presetsman->save();

  lg->info("Goodbye from MainWindow!");
  delete ui;
}
