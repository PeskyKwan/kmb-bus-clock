#!/usr/bin/env bash
# Build and run every host test (tests/*.cpp with c++, tests/*.mjs with node).
# Usage: tests/run_host_tests.sh   (from anywhere; runs from the repo root)
# Exits non-zero if any test fails to build or run.
set -uo pipefail

cd "$(dirname "$0")/.."

AJ_VERSION=6.21.5
AJ_URL="https://github.com/bblanchon/ArduinoJson/releases/download/v${AJ_VERSION}/ArduinoJson-v${AJ_VERSION}.h"
AJ_SHA256=47eca7985cf619dfa1a60d5cc1a6b3b7213a233bd45119eb4af56dd4f12e1962
DEPS_DIR=.host-test-deps

# ArduinoJson: prefer the PlatformIO copy, otherwise fetch the pinned single header.
if [ -f .pio/libdeps/board/ArduinoJson/src/ArduinoJson.h ]; then
  AJ_INCLUDE=.pio/libdeps/board/ArduinoJson/src
else
  AJ_INCLUDE="$DEPS_DIR/ArduinoJson-$AJ_VERSION"
  if [ ! -f "$AJ_INCLUDE/ArduinoJson.h" ]; then
    echo "Downloading ArduinoJson $AJ_VERSION..."
    mkdir -p "$AJ_INCLUDE"
    tmp="$AJ_INCLUDE/ArduinoJson.h.part"
    if ! curl -fsSL --retry 3 -o "$tmp" "$AJ_URL"; then
      rm -f "$tmp"; echo "FAIL: could not download $AJ_URL" >&2; exit 1
    fi
    if ! echo "$AJ_SHA256  $tmp" | sha256sum -c --quiet -; then
      rm -f "$tmp"; echo "FAIL: ArduinoJson checksum mismatch" >&2; exit 1
    fi
    mv "$tmp" "$AJ_INCLUDE/ArduinoJson.h"
  fi
fi

BUILD_DIR=$(mktemp -d)
trap 'rm -rf "$BUILD_DIR"' EXIT

pass=0
failed=()

for src in tests/*.cpp; do
  name=$(basename "$src" .cpp)
  if c++ -std=c++17 -pthread -I"$AJ_INCLUDE" "$src" -o "$BUILD_DIR/$name" \
     && "$BUILD_DIR/$name"; then
    echo "PASS $src"; pass=$((pass + 1))
  else
    echo "FAIL $src"; failed+=("$src")
  fi
done

for src in tests/*.mjs; do
  if node "$src"; then
    echo "PASS $src"; pass=$((pass + 1))
  else
    echo "FAIL $src"; failed+=("$src")
  fi
done

echo "Host tests: $pass passed, ${#failed[@]} failed"
if [ ${#failed[@]} -ne 0 ]; then
  printf '  %s\n' "${failed[@]}"
  exit 1
fi
