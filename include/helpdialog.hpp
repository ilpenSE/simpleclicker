#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

class HelpDialog : public QDialog {
  Q_OBJECT
public:
  explicit HelpDialog(QWidget *parent = nullptr);

private:
  QVBoxLayout *m_layout;
    QTextBrowser *m_browser;
    QPushButton *m_closeBtn;
};
