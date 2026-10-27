#include "preset_code.hpp"
#include <QByteArray>
#include <cstdint>

static void put_varint(QByteArray &b, uint32_t v)
{
  while (v >= 0x80) { b.append(char((v & 0x7F) | 0x80)); v >>= 7; }
  b.append(char(v));
}

static bool get_varint(const QByteArray &b, int &pos, uint32_t &out)
{
  out = 0;
  for (int shift = 0; shift < 35 && pos < b.size(); shift += 7) {
    auto byte = uint8_t(b[pos++]);
    out |= uint32_t(byte & 0x7F) << shift;
    if (!(byte & 0x80)) return true;
  }
  return false;
}

static uint32_t zigzag(int v) { return (uint32_t(v) << 1) ^ uint32_t(v >> 31); }
static int      unzigzag(uint32_t v) { return int(v >> 1) ^ -int(v & 1); }

QString preset_code_serialize(const QString &name, const PresetConfig &c)
{
  QByteArray b;
  b.append(char(FORMAT_VERSION));
  // 00|100        |1                 |1              |0
  //   |mouseButton|repeatUntilStopped|currentLocation|clickMode
  //   |MouseButton::Button4|true|true|ClickMode::Click
  b.append(char(
                (uint8_t(c.clickMode) & 1)
                  | (c.currentLocation ? 2 : 0)
                  | (c.repeatUntilStopped ? 4 : 0)
                  | (uint8_t(c.mouseButton) << 3)
               ));
  put_varint(b, c.interval);
  put_varint(b, c.holdTime);
  put_varint(b, c.repeat);
  put_varint(b, zigzag(c.location.x));
  put_varint(b, zigzag(c.location.y));
  QByteArray n = name.toUtf8().left(255);
  b.append(char(n.size()));
  b.append(n);
  return FORMAT_PREFIX +
    QString::fromLatin1(b.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

std::optional<std::pair<QString, PresetConfig>> preset_code_deserialize(const QString &code)
{
  if (!code.startsWith(FORMAT_PREFIX)) return std::nullopt;
  auto r = QByteArray::fromBase64Encoding(code.mid(4).toLatin1(),
                                          QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
  if (!r) return std::nullopt;
  QByteArray b = *r;
  int pos = 0;

  if (b.size() < 2 || b[pos++] != FORMAT_VERSION) return std::nullopt;

  uint8_t flags = b[pos++];
  uint32_t interval, hold, repeat, zx, zy;
  if (!get_varint(b, pos, interval) || !get_varint(b, pos, hold) ||
      !get_varint(b, pos, repeat)   || !get_varint(b, pos, zx)   ||
      !get_varint(b, pos, zy)) return std::nullopt;
  if (pos >= b.size()) return std::nullopt;

  int nlen = b[pos++];
  if (pos + nlen != b.size()) return std::nullopt;
  QString name = QString::fromUtf8(b.mid(pos, nlen));

  PresetConfig c;
  c.clickMode          = (flags & 1) ? ClickMode::Hold : ClickMode::Click;
  c.currentLocation    = flags & 2;
  c.repeatUntilStopped = flags & 4;
  int mb = (flags >> 3) & 7;
  if (mb >= int(MouseButton::Count)) return std::nullopt;
  c.mouseButton = MouseButton(mb);
  c.interval = interval; c.holdTime = hold; c.repeat = repeat;
  c.location = { unzigzag(zx), unzigzag(zy) };

  if (name.isEmpty() || !c.validate()) return std::nullopt;
  return std::make_pair(name, c);
}
