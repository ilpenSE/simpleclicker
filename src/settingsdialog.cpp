#include "settingsdialog.hpp"
#include <QFormLayout>
#include "common.hpp"
#include "logger.hpp"
#include "theme.hpp"
#include "settings.hpp"
extern ThemeManager *thememan;
extern SettingsManager *settingsman;
extern Logger *lg;

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Settings");
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
  layout->addRow("Language:", m_languageCombo);

  m_themeCombo = new QComboBox(this);
  m_themeCombo->addItem("Dark", static_cast<int>(Theme::Dark));
  m_themeCombo->addItem("Light", static_cast<int>(Theme::Light));
  layout->addRow("Theme:", m_themeCombo);

  m_hotkeyEdit = new QKeySequenceEdit(this);
  layout->addRow("Start/Stop Hotkey:", m_hotkeyEdit);

  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  layout->addRow(m_buttonBox);
  outerLayout->addWidget(formContainer);

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onSave);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  // Load language
  m_currentLang = settingsman->get<Language>("language");
  int idx = m_languageCombo->findData(static_cast<int>(m_currentLang));
  if (idx >= 0) m_languageCombo->setCurrentIndex(idx);

  // Load theme
  m_currentTheme = settingsman->get<Theme>("theme");
  idx = m_themeCombo->findData(static_cast<int>(m_currentTheme));
  if (idx >= 0) m_themeCombo->setCurrentIndex(idx);

  // Load hotkey
  m_currentHotkey = settingsman->get<QString>("keybind");
  m_hotkeyEdit->setKeySequence(QKeySequence(m_currentHotkey));
}

void SettingsDialog::onSave() {
  bool madeChanges = false;

  auto selectedHotkey = m_hotkeyEdit->keySequence().toString();
  if (selectedHotkey != m_currentHotkey) {
    if (selectedHotkey == SAVE_CHANGES_KEYBIND ||
        selectedHotkey == ABORT_CHANGES_KEYBIND) {
      m_notificationBar->error(QString("Cannot change hotkey because it's in already use, try something else other than %1").arg(selectedHotkey));
      lg->error("Cannot change hotkey because it's reserved for program");
      m_hotkeyEdit->setKeySequence(m_currentHotkey);
      return;
    } else {
      madeChanges = true;
      settingsman->set("keybind", selectedHotkey);
      // TODO: Add hotkey manager's change/set hotkey function here
    }
  }

  Language selectedLanguage = static_cast<Language>(m_languageCombo->currentData().toInt());
  if (selectedLanguage != m_currentLang) {
    madeChanges = true;
    settingsman->set<Language>("language", selectedLanguage);
    // TODO: Add language manager's change/set language function here
  }

  Theme selectedTheme = static_cast<Theme>(m_themeCombo->currentData().toInt());
  if (selectedTheme != m_currentTheme) {
    madeChanges = true;
    ThemeManager::instance().setTheme(selectedTheme);
    settingsman->set<Theme>("theme", selectedTheme);
  }

  if (madeChanges) emit settingsApplied();
  accept();
}
