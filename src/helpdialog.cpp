#include "helpdialog.hpp"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFile>
#include <QIODevice>
#include "common.hpp"
#include "logger.hpp"
#include "language.hpp"
extern LanguageManager *langman;
extern Logger *lg;

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Help Menu"));
  resize(800, 600);

  auto lang = langman->language();
  QString localizedReadme;
  if (lang != Language::English) {
    localizedReadme = QString(":/docs/readme.%1.md").arg(to_cstr(lang));
  } else localizedReadme = ":/docs/readme.md";

  m_layout = new QVBoxLayout(this);

  m_browser = new QTextBrowser(this);
  m_browser->setOpenExternalLinks(true);
  QFile readmeFile(localizedReadme);
  if (readmeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString md = QString::fromUtf8(readmeFile.readAll());
    m_browser->setMarkdown(md);
  } else {
    m_browser->setMarkdown(tr("Help content couldn't be loaded"));
    lg->error("{} couldn't be loaded", localizedReadme);
  }

  m_closeBtn = new QPushButton(tr("Close"), this);
  makeDynamicIconButton(m_closeBtn, "cancel.svg");

  connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

  m_layout->addWidget(m_browser);
  m_layout->addWidget(m_closeBtn, 0, Qt::AlignRight);
}
