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

namespace {
constexpr QStringView kBeta = u"-beta";
constexpr QStringView kRc   = u"-rc";
}

Version Version::from(QStringView sv) {
  bool is_beta = sv.endsWith(kBeta);
  if (is_beta) sv.chop(kBeta.size());
  bool is_rc = sv.endsWith(kRc);
  if (is_rc) sv.chop(kRc.size());

  Version res;
  res.channel = is_beta ? VersionChannel::Beta
              : is_rc ? VersionChannel::ReleaseCandidate
                : VersionChannel::Release;

  QStringView rest = sv;
  for (int i = 0; i < 3; ++i) {
    qsizetype dot = rest.indexOf(u'.');
    QStringView part = (dot == -1) ? rest : rest.first(dot);

    bool ok = false;
    int val = part.toInt(&ok);
    if (!ok || val < 0 || val > 255) return {};

    if (i == 0) res.major = static_cast<uint8_t>(val);
    else if (i == 1) res.minor = static_cast<uint8_t>(val);
    else res.patch = static_cast<uint8_t>(val);

    if (dot == -1) break;
    rest = rest.sliced(dot + 1);
  }
  return res;
}
