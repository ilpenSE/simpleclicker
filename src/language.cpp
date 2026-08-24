#include "language.hpp"
#include "logger.hpp"
extern Logger *lg;
#include <QApplication>
#include <QLibraryInfo>

void LanguageManager::set(Language newLanguage, bool force) {
  if (m_lang == newLanguage && !force) return;
  qApp->removeTranslator(&m_translator);
  qApp->removeTranslator(&m_qtTranslator);

  // Load our translator
  bool loaded = m_translator.load(QString(":/translations/%1.qm").arg(to_cstr(newLanguage)));
  if (loaded) {
    if (!qApp->installTranslator(&m_translator)) {
      lg->error("Failed to install translator for {} language", newLanguage);
      return;
    }
    m_lang = newLanguage;
    emit languageChanged();
  } else {
    lg->error("Failed to load language file: {}.qm", newLanguage);
  }
}
