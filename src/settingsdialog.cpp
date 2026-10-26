#include "settingsdialog.hpp"
#include <QFormLayout>
#include "common.hpp"
#include "logger.hpp"
#include "theme.hpp"
#include "settings.hpp"
#include "language.hpp"
#include "hotkey.hpp"
#include "build_info.hpp"

extern ThemeManager *thememan;
extern SettingsManager *settingsman;
extern Logger *lg;
extern LanguageManager *langman;
extern HotkeyManager *hotkeyman;

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
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

  // Language
  m_languageCombo = new QComboBox(this);
  m_languageCombo->addItem("English", static_cast<int>(Language::English));
  m_languageCombo->addItem("Türkçe", static_cast<int>(Language::Turkish));
  m_languageCombo->addItem("Deutsch", static_cast<int>(Language::German));
  m_languageCombo->addItem("Français", static_cast<int>(Language::French));
  layout->addRow(tr("Language"), m_languageCombo);

  // Theme
  m_themeCombo = new QComboBox(this);
  m_themeCombo->addItem(tr("Dark"), static_cast<int>(Theme::Dark));
  m_themeCombo->addItem(tr("Light"), static_cast<int>(Theme::Light));
  layout->addRow(tr("Theme"), m_themeCombo);

  // Hotkey
  m_hotkeyEdit = new QKeySequenceEdit(this);
  layout->addRow(tr("Start/Stop Hotkey"), m_hotkeyEdit);

  outerLayout->addWidget(formContainer);

  // Constraint hotkey edit
  m_hotkeyEdit->setMaximumSequenceLength(1);
  connect(m_hotkeyEdit, &QKeySequenceEdit::editingFinished, this, [this]() {
    QKeySequence seq = m_hotkeyEdit->keySequence();
    auto hk = Hotkey::from(seq);

    // If given keybind is invalid/unsupported/reserved, rollback to old one
    if (hk.key == 0 || !HotkeyManager::isSupportedKey(hk.key)
        || hk == SAVE_CHANGES_KEYBIND || hk == ABORT_CHANGES_KEYBIND) {
      m_notificationBar->error(tr("Keybind is invalid, try something else"));
      lg->error("Cannot change hotkey ({}) because it's invalid", seq.toString());
      m_hotkeyEdit->setKeySequence(m_selectedHotkey.toKeySequence());
    }
  });

  // Save/cancel buttons
  auto btnLayout = new QHBoxLayout();
  m_saveBtn = new QPushButton(tr("Save"), this);
  m_saveBtn->setCursor(Qt::PointingHandCursor);
  m_cancelBtn = new QPushButton(tr("Cancel"), this);
  m_cancelBtn->setCursor(Qt::PointingHandCursor);
  makeDynamicIconButton(m_saveBtn, "save.svg");
  makeDynamicIconButton(m_cancelBtn, "cancel.svg");

  btnLayout->addStretch();
  btnLayout->setSpacing(8);
  btnLayout->setContentsMargins(6,6,16,16);
  btnLayout->addWidget(m_saveBtn);
  btnLayout->addWidget(m_cancelBtn);
  outerLayout->addLayout(btnLayout);

  auto infoLayout = new QHBoxLayout();
  infoLayout->setContentsMargins(10,0,10,10);
  auto infoLabel = new QLabel(SC_BUILD_TIME ", commit " SC_GIT_HASH ", " SC_COMPILER_VERSION ", Qt " SC_QT_VERSION);
  infoLabel->setEnabled(false);
  infoLabel->setToolTip(tr("Build date, commit, compiler with version and Qt version"));
  infoLayout->addWidget(infoLabel);
  outerLayout->addLayout(infoLayout);

  connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
  connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  // Load language
  m_currentLang = settingsman->get<setting::language>();
  m_languageCombo->setCurrentIndex(static_cast<int>(m_currentLang));

  // Load theme
  m_currentTheme = settingsman->get<setting::theme>();
  m_themeCombo->setCurrentIndex(static_cast<int>(m_currentTheme));

  // Load hotkey
  m_currentHotkey = settingsman->get<setting::keybind>();
  m_hotkeyEdit->setKeySequence(m_currentHotkey.toKeySequence());
  m_selectedHotkey = m_currentHotkey;
}

void SettingsDialog::onSave() {
  bool madeChanges = false;

  // Hotkey
  auto selectedHotkey = Hotkey::from(m_hotkeyEdit->keySequence());
  if (selectedHotkey != m_currentHotkey) {
    if (selectedHotkey == SAVE_CHANGES_KEYBIND ||
        selectedHotkey == ABORT_CHANGES_KEYBIND) {
      m_notificationBar->error(tr("%1 is reserved for program, please try something else").arg(selectedHotkey.toString()));
      lg->error("Cannot change hotkey ({}) because it's reserved for program", selectedHotkey.toString());
      m_hotkeyEdit->setKeySequence(m_currentHotkey.toKeySequence());
      return;
    } else {
      madeChanges = true;
      settingsman->set<setting::keybind>(selectedHotkey);
      hotkeyman->set(selectedHotkey);
    }
  } else {
    hotkeyman->set(m_currentHotkey);
  }

  // Language
  Language selectedLanguage = static_cast<Language>(m_languageCombo->currentIndex());
  if (selectedLanguage != m_currentLang) {
    madeChanges = true;
    langman->set(selectedLanguage);
    settingsman->set<setting::language>(selectedLanguage);
  }

  // Theme
  Theme selectedTheme = static_cast<Theme>(m_themeCombo->currentIndex());
  if (selectedTheme != m_currentTheme) {
    madeChanges = true;
    thememan->setTheme(selectedTheme);
    settingsman->set<setting::theme>(selectedTheme);
  }

  if (madeChanges) emit settingsApplied();
  accept();
}
