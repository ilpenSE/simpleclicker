#include "helpdialog.hpp"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFile>
#include <QIODevice>
#include "common.hpp"

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Help");
  resize(800, 600);

  m_layout = new QVBoxLayout(this);

  m_browser = new QTextBrowser(this);
  m_browser->setOpenExternalLinks(true);
  QFile readmeFile(":/readme.md");
  if (readmeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString md = QString::fromUtf8(readmeFile.readAll());
    m_browser->setMarkdown(md);
  } else
    m_browser->setPlainText("Help content couldn't be loaded");

  m_closeBtn = new QPushButton("Close", this);
  makeDynamicIconButton(m_closeBtn, "cancel.svg");

  connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

  m_layout->addWidget(m_browser);
  m_layout->addWidget(m_closeBtn, 0, Qt::AlignRight);
}
