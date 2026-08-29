#include "common.hpp"
#include "theme.hpp"
extern ThemeManager *thememan;

void makeDynamicIconButton(QPushButton *btn, const QString &symbol) {
  btn->setIcon(thememan->icon(symbol));
  if (btn->text() == "") btn->setStyleSheet(R"(QPushButton { background-color: none; border: none; }
                                             QPushButton:hover { border: 1px solid #4287f5; } )");
  QObject::connect(thememan, &ThemeManager::themeChanged, btn, [btn, symbol](){
    btn->setIcon(thememan->icon(symbol));
  });
}

Version Version::from(QStringView sv) {
  bool is_beta = sv.endsWith(u"-beta");
  if (is_beta) sv.chop(5);

  Version res;
  res.is_beta = is_beta;

  QStringView rest = sv;
  for (int i = 0; i < 3; ++i) {
    qsizetype dot = rest.indexOf(u'.');
    QStringView part = (dot == -1) ? rest : rest.first(dot);

    bool ok = false;
    int val = part.toInt(&ok);
    if (!ok) return {};

    if (i == 0) res.major = static_cast<uint8_t>(val);
    else if (i == 1) res.minor = static_cast<uint8_t>(val);
    else res.patch = static_cast<uint8_t>(val);

    if (dot == -1) break;
    rest = rest.sliced(dot + 1);
  }
  return res;
}
