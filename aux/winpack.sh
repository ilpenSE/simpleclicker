#!/bin/bash
set -euo pipefail

ROOT="${1:?Kullanım: $0 <exe-or-dll>}"
DEST="${2:-$(dirname "$ROOT")}"

declare -A seen

resolve_dll() {
  local dll="$1"

  # Already seen
  [[ ${seen["$dll"]+x} ]] && return
  seen["$dll"]=1

  # Skip windows system DLLs
  case "${dll,,}" in
    kernel32.dll|user32.dll|gdi32.dll|advapi32.dll|shell32.dll|ole32.dll|oleaut32.dll|\
      ws2_32.dll|comdlg32.dll|comctl32.dll|rpcrt4.dll|secur32.dll|bcrypt.dll|\
      ntdll.dll|ntuser.dll|imm32.dll|version.dll|authz.dll|mpr.dll|netapi32.dll|userenv.dll|\
      winmm.dll|d3d11.dll|d3d12.dll|dwrite.dll|dxgi.dll|uxtheme.dll|\
      api-ms-win-*.dll|ext-ms-win-*.dll|dwmapi.dll)
      return
      ;;
  esac

  local path=""

  # Find DLL from PATH
  IFS=: read -ra paths <<< "$PATH"
  for dir in "${paths[@]}"; do
    if [[ -f "$dir/$dll" ]]; then
      path="$dir/$dll"
      break
    fi
  done

  if [[ -z "$path" ]]; then
    echo "[MISSING] $dll"
    return
  fi

  cp "$path" "$DEST"
  echo "$path"

  while read -r dep; do
    resolve_dll "$dep"
  done < <(
    x86_64-w64-mingw32-objdump -p "$path" |
      sed -n 's/^[[:space:]]*DLL Name: //p'
  )
}

while read -r dep; do
  resolve_dll "$dep"
done < <(
  x86_64-w64-mingw32-objdump -p "$ROOT" |
    sed -n 's/^[[:space:]]*DLL Name: //p'
)
