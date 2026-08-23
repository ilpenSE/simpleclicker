#pragma once

#include <QDialog>

#include <QKeySequenceEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include "notificationbar.hpp"
#include "common.hpp"

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget *parent = nullptr);

signals:
  void settingsApplied();

private slots:
  void onSave();

private:
  Language m_currentLang;
  Theme m_currentTheme;
  QString m_currentHotkey;

  QComboBox *m_languageCombo;
  QComboBox *m_themeCombo;
  QKeySequenceEdit *m_hotkeyEdit;
  QDialogButtonBox *m_buttonBox;
  NotificationBar *m_notificationBar;
};
