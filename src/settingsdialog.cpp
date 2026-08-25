#include "settingsdialog.hpp"
#include <QFormLayout>
#include "common.hpp"
#include "logger.hpp"
#include "theme.hpp"
#include "settings.hpp"
#include "language.hpp"
#include "hotkey.hpp"

extern ThemeManager *thememan;
extern SettingsManager *settingsman;
extern Logger *lg;
extern LanguageManager *langman;
extern HotkeyManager *hotkeyman;

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Settings"));
  setModal(true);
  setMinimumWidth(360);
  setMinimumHeight(200);

  auto outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);

  m_notificationBar = new NotificationBar(this);
  outerLayout->addWidget(m_notificationBar);

  auto *formContainer = new QWidget(this);
  auto layout = new QFormLayout(formContainer);
  layout->setContentsMargins(16, 16, 16, 16);

  m_languageCombo = new QComboBox(this);
  m_languageCombo->addItem("English", static_cast<int>(Language::English));
  m_languageCombo->addItem("Türkçe", static_cast<int>(Language::Turkish));
  layout->addRow(tr("Language"), m_languageCombo);

  m_themeCombo = new QComboBox(this);
  m_themeCombo->addItem(tr("Dark"), static_cast<int>(Theme::Dark));
  m_themeCombo->addItem(tr("Light"), static_cast<int>(Theme::Light));
  layout->addRow(tr("Theme"), m_themeCombo);

  m_hotkeyEdit = new QKeySequenceEdit(this);
  layout->addRow(tr("Start/Stop Hotkey"), m_hotkeyEdit);
  outerLayout->addWidget(formContainer);

  // Save/cancel buttons
  auto btnLayout = new QHBoxLayout();
  m_saveBtn = new QPushButton(tr("Save"), this);
  m_cancelBtn = new QPushButton(tr("Cancel"), this);
  makeDynamicIconButton(m_saveBtn, "save.svg");
  makeDynamicIconButton(m_cancelBtn, "cancel.svg");

  btnLayout->addStretch();
  btnLayout->setSpacing(8);
  btnLayout->setContentsMargins(6,6,16,16);
  btnLayout->addWidget(m_saveBtn);
  btnLayout->addWidget(m_cancelBtn);
  outerLayout->addLayout(btnLayout);

  connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
  connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  // Load language
  m_currentLang = settingsman->get<language>();
  int idx = m_languageCombo->findData(static_cast<int>(m_currentLang));
  if (idx >= 0) m_languageCombo->setCurrentIndex(idx);

  // Load theme
  m_currentTheme = settingsman->get<theme>();
  idx = m_themeCombo->findData(static_cast<int>(m_currentTheme));
  if (idx >= 0) m_themeCombo->setCurrentIndex(idx);

  // Load hotkey
  m_currentHotkey = settingsman->get<keybind>();
  m_hotkeyEdit->setKeySequence(m_currentHotkey.toKeySequence());
}

void SettingsDialog::onSave() {
  bool madeChanges = false;

  auto selectedHotkey = Hotkey::from(m_hotkeyEdit->keySequence());
  if (selectedHotkey != m_currentHotkey) {
    if (selectedHotkey == SAVE_CHANGES_KEYBIND ||
        selectedHotkey == ABORT_CHANGES_KEYBIND) {
      m_notificationBar->error(QString(tr("%1 is reserved for program, please try something else")).arg(selectedHotkey.toString()));
      lg->error("Cannot change hotkey ({}) because it's reserved for program", selectedHotkey.toString());
      m_hotkeyEdit->setKeySequence(m_currentHotkey.toKeySequence());
      return;
    } else {
      madeChanges = true;
      settingsman->set<keybind>(selectedHotkey);
      hotkeyman->unset();
      hotkeyman->set(selectedHotkey);
    }
  }

  Language selectedLanguage = static_cast<Language>(m_languageCombo->currentData().toInt());
  if (selectedLanguage != m_currentLang) {
    madeChanges = true;
    langman->set(selectedLanguage);
    settingsman->set<language>(selectedLanguage);
  }

  Theme selectedTheme = static_cast<Theme>(m_themeCombo->currentData().toInt());
  if (selectedTheme != m_currentTheme) {
    madeChanges = true;
    thememan->setTheme(selectedTheme);
    settingsman->set<theme>(selectedTheme);
  }

  if (madeChanges) emit settingsApplied();
  accept();
}
