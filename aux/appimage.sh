#!/bin/bash
# Usage: ./appimage.sh [build_dir]
# "appimagetool" should be in the path
set -euo pipefail

QT="${QT:-$HOME/Qt/6.10.3/gcc_64}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$(readlink -f "${1:-$ROOT/build}")"
APP="SimpleClicker"
APPDIR="$BUILD/AppDir"
OUT="$BUILD/$APP-x86_64.AppImage"

PLUGINS=(
  platforms/libqxcb.so
  imageformats/libqsvg.so
  imageformats/libqjpeg.so
  imageformats/libqgif.so
  iconengines/libqsvgicon.so
)
PLUGIN_DIRS=(xcbglintegrations tls)

# Sistemden paketlenecek kütüphaneler (ana libxcb hariç)
BUNDLE_SYS='^(libxcb-.*|libxkbcommon.*|libssl|libcrypto)\.so'
# dlopen ile yüklenenler, ldd'de görünmez
EXTRA_SYS=(libxcb-cursor.so.0 libssl.so.3 libcrypto.so.3)

[ -x "$BUILD/$APP" ] || { echo "Binary yok: $BUILD/$APP"; exit 1; }
command -v appimagetool >/dev/null || { echo "appimagetool bulunamadı"; exit 1; }

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/plugins"

cp "$BUILD/$APP" "$APPDIR/usr/bin/"
for p in "${PLUGINS[@]}"; do
  mkdir -p "$APPDIR/usr/plugins/$(dirname "$p")"
  cp -L "$QT/plugins/$p" "$APPDIR/usr/plugins/$(dirname "$p")/"
done
for d in "${PLUGIN_DIRS[@]}"; do
  mkdir -p "$APPDIR/usr/plugins/$d"
  cp -L "$QT/plugins/$d"/* "$APPDIR/usr/plugins/$d/"
done

# Bağımlılıkları kapanış bulunana kadar tara
declare -A SEEN
queue=("$APPDIR/usr/bin/$APP")
while IFS= read -r f; do queue+=("$f"); done \
  < <(find "$APPDIR/usr/plugins" -name '*.so')

for extra in "${EXTRA_SYS[@]}"; do
  src="$(ldconfig -p | awk -v n="$extra" '$1==n && /x86-64/ && !d {print $NF; d=1}')"
  [ -n "$src" ] || { echo "UYARI: $extra bulunamadı"; continue; }
  cp -L "$src" "$APPDIR/usr/lib/"
  SEEN[$extra]=1
  queue+=("$APPDIR/usr/lib/$extra")
done

while [ ${#queue[@]} -gt 0 ]; do
  f="${queue[0]}"; queue=("${queue[@]:1}")
  while read -r path; do
    [ -n "$path" ] || continue
    base="$(basename "$path")"
    [[ "$path" == "$APPDIR"/* ]] && continue
    [ -n "${SEEN[$base]:-}" ] && continue
    if [[ "$path" == "$QT"/* ]] || [[ "$base" =~ $BUNDLE_SYS ]]; then
      SEEN[$base]=1
      cp -L "$path" "$APPDIR/usr/lib/"
      queue+=("$APPDIR/usr/lib/$base")
    fi
  done < <(LD_LIBRARY_PATH="$QT/lib" ldd "$f" | awk '/=> \// {print $3}')
done

# AppRun, desktop, ikon
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$HERE/usr/plugins"
exec "$HERE/usr/bin/SimpleClicker" "$@"
EOF
chmod +x "$APPDIR/AppRun"

cp "$ROOT/assets/SimpleClicker.desktop" "$APPDIR/"
cp "$ROOT/assets/icons/app.png" "$APPDIR/simpleclicker.png"

ARCH=x86_64 appimagetool "$APPDIR" "$OUT"

echo
echo "Libraries:"
ls -1 "$APPDIR/usr/lib"
echo
echo "Output: $OUT"
