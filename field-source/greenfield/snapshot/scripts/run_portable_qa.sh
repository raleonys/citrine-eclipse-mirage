#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
for compiler in g++ clang++; do
  command -v "$compiler" >/dev/null || { echo "MISSING_COMPILER:$compiler"; exit 2; }
  cfg="${ROOT}/_local_portable_${compiler//+/x}"
  cmake -S "$ROOT" -B "$cfg" -G Ninja -DGREENFIELD_PORTABLE_QA_ONLY=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$(command -v "$compiler")" \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" >/dev/null
  cmake --build "$cfg" >/dev/null
  ctest --test-dir "$cfg" --output-on-failure
  echo "PASS_PORTABLE_GF02:${compiler}"
done
