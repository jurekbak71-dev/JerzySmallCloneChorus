#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
CMAKE=/workspace/toolchains/python/cmake/data/bin/cmake
CTEST=/workspace/toolchains/python/cmake/data/bin/ctest
NINJA=/workspace/toolchains/python/bin/ninja
"$CMAKE" -S . -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="$NINJA"
"$CMAKE" --build build-linux --parallel 4
"$CTEST" --test-dir build-linux --output-on-failure
"$CMAKE" -S . -B build-win -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="$NINJA" -DCMAKE_TOOLCHAIN_FILE=cmake/mingw.cmake
"$CMAKE" --build build-win --parallel 4
python3 tools/package.py --build build-win
