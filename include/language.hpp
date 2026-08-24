#pragma once

#include "common.hpp"
#include <QTranslator>
#include <QObject>

class LanguageManager : public QObject {
  Q_OBJECT
public:
  static LanguageManager& instance(Language initialLanguage = Language::English) {
    static LanguageManager inst(initialLanguage);
    return inst;
  }

  Language language() { return m_lang; }
  void set(Language newLanguage, bool force = false);

  LanguageManager(const LanguageManager&) = delete;
  LanguageManager operator=(const LanguageManager&) = delete;

signals:
  void languageChanged();

private:
  Language m_lang = Language::English;
  QTranslator m_translator{};
  QTranslator m_qtTranslator{};
  LanguageManager(Language initialLanguage, QObject *parent = nullptr)
      : m_lang(initialLanguage), QObject(parent) {
    set(initialLanguage, true);
  }
  ~LanguageManager() {}
};
