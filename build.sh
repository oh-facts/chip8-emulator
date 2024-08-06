#!/bin/bash
cd "${0%/*}"

for arg in "$@"; do eval "$arg=1"; done

common_c_flags="-std=c99 -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-write-strings -Wno-comment -Wno-misleading-indentation -Wno-unused-result -g"
common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-write-strings -Wno-comment -Wno-misleading-indentation -Wno-unused-result"

debug_build="-O0 -g"
release_build="-O3"

[ "$cloc" == "1" ] && cloc --exclude-list-file=.clocignore "code/" && exit 0
[ "$debug" == "1" ] && build_type="$debug_build" 
[ "$release" == "1" ] && build_type="$release_build"
[ "$clang" == "1" ] && compiler_type="clang" && echo "[clang]"
[ "$gcc" == "1" ] && compiler_type="gcc" && echo "[gcc]"

[ "$clean" == "1" ] && echo "deleted /out && code/meta.h" && rm -rf "out/" && rm "code/meta.h"

[ "$build_type" == "$debug_build" ] && echo "[debug build]"
[ "$build_type" == "$release_build" ] && echo "[release build]"

[ ! -d "out" ] && mkdir "out" && echo "[created build dir]"

[ "$metacr" == "1" ] && echo "[generating meta.h]" && $compiler_type $common_c_flags $debug_build code/cpp.c -o out/meta && out/meta > code/meta.h

[ "$platform" == "1" ] && $compiler_type $common_flags $build_type code/main.cpp -lSDL3 -lm -I./code/ -o "out/platform" && echo "compiled platform"

[ "$app" == "1" ] && $compiler_type $common_flags $build_type -fPIC -shared -lm code/saoirse.cpp -o "out/libyk.so" && echo "compiled app"

[ "$run" == "1" ] && ./out/platform