#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.hpp"
#include "logger.hpp"
#include "settings.hpp"
#include "theme.hpp"
#include "language.hpp"
#include "presetitemwidget.hpp"
#include <QShortcut>
#include <QDesktopServices>
#include <QUrl>
#include "helpdialog.hpp"
#include "settingsdialog.hpp"
#include "hotkey.hpp"
#include "click.hpp"
#include "locationpickeroverlay.hpp"

extern Logger *lg;
extern PresetManager *presetsman;
extern SettingsManager *settingsman;
extern ThemeManager *thememan;
extern LanguageManager *langman;
extern HotkeyManager *hotkeyman;
extern ClickEngine *clickengine;

static PresetItemWidget *currentPresetWidget = nullptr;
static bool presetChangeEventLock = false;
static bool uiConstructed = false;

MainWindow::MainWindow(QString initPreset, QWidget *parent)
  : QMainWindow(parent) , m_currentPreset(initPreset), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  m_notificationBar = new NotificationBar(this);
  qobject_cast<QVBoxLayout *>(centralWidget()->layout())->insertWidget(0, m_notificationBar);

  lg->info("MainWindow initialized");

  // Set up theme manager
  connect(thememan, &ThemeManager::themeChanged, this, &MainWindow::applyTheme);
  applyTheme(thememan->theme());

  // Subscribe to language changes
  connect(langman, &LanguageManager::languageChanged, this, &MainWindow::retranslateUi);
  retranslateUi();

  // Add presets to the list
  for (const auto& [name, cfg] : presetsman->presets.asKeyValueRange()) {
    addPresetItem(*ui->presetsList, name, cfg);
  }

  applySettings();

  // Bind Save keybind
  auto *saveShortcut = new QShortcut(SAVE_CHANGES_KEYBIND.toKeySequence(), this);
  connect(saveShortcut, &QShortcut::activated, this, [this]() {
    if (currentPresetWidget) emit currentPresetWidget->saveRequested();
  });

  // Bind Cancel keybind
  auto *cancelShortcut = new QShortcut(ABORT_CHANGES_KEYBIND.toKeySequence(), this);
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
    m_notificationBar->success(QString(tr("Created new preset called %1, you can rename it now")).arg(newPresetName));
  });

  // If there's no current preset, block the preset config UI
  if (!currentPresetWidget) {
    blockPresetConfigUi();

    if (presetsman->presets.size() == 0) {
      m_notificationBar->info(tr("You can create a new preset using (+) button."), 5 * 60 * 1000);
    } else {
      m_notificationBar->info(tr("You can select a preset from left panel."), 5 * 60 * 1000);
    }
  }

  // Set up settings button
  connect(ui->settingsBtn, &QPushButton::clicked, this, [this]() {
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::settingsApplied, this, [this]() {
      applySettings();
      lg->info("Settings changed and applied.");
      m_notificationBar->success(tr("Settings has been applied successfully!"));
    });
    dlg.exec();
  });

  // Subscribe to start/stop buttons
  connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startClicking);
  connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopClicking);

  // Subscribe to hotkey event
  connect(hotkeyman, &HotkeyManager::hotkeyPressed, this, [this]() {
    if (clickengine->running) stopClicking(); else startClicking();
  });
  connect(hotkeyman, &HotkeyManager::hotkeyRegistrationFailed, this, [this]() {
    m_notificationBar->error(tr("Start/Stop hotkey couldn't be registered please restart or reinstall the app"));
    lg->error("Hotkey registeration failed!");
  });

  // Show internal embedded readme to user with a dialog
  connect(ui->helpBtn, &QPushButton::clicked, this, [this]() {
    HelpDialog dlg(this);
    dlg.exec();
  });

  // Subscribe to stop events of click engine
  connect(clickengine, &ClickEngine::clickFinished, this, &MainWindow::stopClicking, Qt::QueuedConnection);

  // Set up location picker
  connect(ui->pickLocationButton, &QPushButton::clicked, this, &MainWindow::showLocationPicker);

  ui->stopButton->setEnabled(false);
  uiConstructed = true;
}

void MainWindow::showLocationPicker() {
  auto picker = new LocationPickerOverlay();
  connect(picker, &LocationPickerOverlay::locationPicked, this, [this](QPoint pos) {
    lg->info("Picked location: x: {}, y: {}", pos.x(), pos.y());
    ui->xEdit->setValue(pos.x());
    ui->yEdit->setValue(pos.y());
    ui->pickLocationRadio->setChecked(true);
    ui->currentLocationRadio->setChecked(false);
  });

  connect(picker, &LocationPickerOverlay::cancelled, this, [this]() {
    lg->info("Location picking cancelled");
  });
  picker->show();
}

void MainWindow::startClicking() {
  savePreset(currentPresetWidget);
  blockPresetConfigUi();
  ui->presetsList->setEnabled(false);
  ui->addPresetBtn->setEnabled(false);

  ui->startButton->setEnabled(false);
  ui->stopButton->setEnabled(true);
  clickengine->start();
  lg->info("Clicking started");
};

void MainWindow::stopClicking() {
  unblockPresetConfigUi();
  ui->startButton->setEnabled(true);
  ui->stopButton->setEnabled(false);

  ui->presetsList->setEnabled(true);
  ui->addPresetBtn->setEnabled(true);
  clickengine->stop();
  lg->info("Clicking stopped");
};

void MainWindow::retranslateUi() {
  ui->retranslateUi(this);
  ui->infoLabel->setText(ui->infoLabel->text().arg(APP_VERSION.toQString()));
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

  if (!currentPresetWidget && presetName == m_currentPreset) {
    setActivePreset(item);
  }

  // Delete an item from preset
  connect(itemWidget, &PresetItemWidget::deleteRequested, this,
          [this, &list, item, itemWidget]() {
    if (!itemWidget->isEnabled()) return;
    lg->info("Removing preset: '{}'", itemWidget->presetName());
    presetsman->presets.remove(itemWidget->presetName());
    int idx = list.row(item);
    if (currentPresetWidget == itemWidget) {
      // if current's gonna be deleted, set active preset to something else
      setActivePreset(idx <= 0 ? nullptr : list.item(idx - 1));
    }
    m_notificationBar->success(QString(tr("Removed preset '%1'.")).arg(itemWidget->presetName()));

    list.removeItemWidget(item);
    delete list.takeItem(idx);
  });

  // Rename the preset name
  connect(itemWidget, &PresetItemWidget::renameRequested, this, [itemWidget, this](const QString &newName) {
    if (!itemWidget->isEnabled()) return;
    lg->info("Renaming preset: '{}' -> '{}'", itemWidget->presetName(), newName);
    if (presetsman->presets.contains(newName)) {
      m_notificationBar->warning(QString(tr("Preset '%1' already exists, rename aborted")).arg(newName));
      lg->warning("Preset '{}' already exists, rename aborted", newName);
      return;
    }

    auto cfg = presetsman->presets.take(itemWidget->presetName());
    presetsman->presets.insert(newName, cfg);
    m_notificationBar->success(QString(tr("Renamed preset '%1' to '%2'")).arg(itemWidget->presetName(), newName));
    itemWidget->setPresetName(newName);
  });

  // Abort changes, cancel requested
  connect(itemWidget, &PresetItemWidget::cancelRequested, this, [this, itemWidget]() {
    if (!itemWidget->isEnabled() || !itemWidget->isUnsaved()) return;
    if (currentPresetWidget) applyPreset(currentPresetWidget->config);
    itemWidget->markSaved();
    m_notificationBar->info(tr("Changes aborted."));
  });

  // Save preset
  connect(itemWidget, &PresetItemWidget::saveRequested, this,
          [this, itemWidget]() {
    savePreset(itemWidget);
  });

  // Double clicked, set current preset to this
  connect(itemWidget, &PresetItemWidget::doubleClicked, this,
          [this, item, itemWidget]() {
    if (!itemWidget->isEnabled()) return;
    setActivePreset(item);
    m_notificationBar->success(QString(tr("Set active preset to %1")).arg(itemWidget->presetName()));
  });

  return item;
}

void MainWindow::savePreset(PresetItemWidget *itemWidget) {
  if (!itemWidget) return;
  if (!itemWidget->isEnabled() || !itemWidget->isUnsaved()) return;
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
  clickengine->setPreset(newConfig);
  m_notificationBar->success(QString(tr("Preset %1 has been saved successfully!")).arg(pname));
}

void MainWindow::_changePresetConfigUi(bool is_locked) {
  for (auto it : {ui->clickOptionsBox, ui->clickIntervalBox, ui->repeatBox, ui->positionBox}) {
    it->setEnabled(!is_locked);
  }
  ui->startButton->setEnabled(!is_locked);
  ui->stopButton->setEnabled(!is_locked);
}

void MainWindow::applySettings() {
  auto kbd = settingsman->get<keybind>().toString();
  ui->startButton->setText(QString(tr("Start") + " (%1)").arg(kbd));
  ui->stopButton->setText(QString(tr("Stop") + " (%1)").arg(kbd));
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

  clickengine->setPreset(config);
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
    candidate = QString(tr("New Preset %1")).arg(n++);
  } while (presetsman->presets.contains(candidate));
  return candidate;
}

void MainWindow::applyTheme(Theme newTheme) {
  if (!uiConstructed) return;
  lg->info("Applied theme: '{}'", newTheme);
}

MainWindow::~MainWindow()
{
  settingsman->set<currentPreset>(currentPresetWidget ? currentPresetWidget->presetName() : "");
  settingsman->set<version>(APP_VERSION);

  settingsman->save();
  presetsman->save();

  lg->info("Goodbye from MainWindow!");
  delete ui;
}
