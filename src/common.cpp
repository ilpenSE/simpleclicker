#include "common.hpp"
#include "theme.hpp"
extern ThemeManager *thememan;

void makeDynamicIconButton(QPushButton *btn, const QString &symbol) {
  btn->setIcon(thememan->icon(symbol));
  QObject::connect(thememan, &ThemeManager::themeChanged, btn, [btn, symbol](){
    btn->setIcon(thememan->icon(symbol));
  });
}
