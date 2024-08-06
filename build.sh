#!/bin/bash
cd "${0%/*}"

for arg in "$@"; do eval "$arg=1"; done

common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment"

debug_build="-O0 -g"
release_build="-O3"

[ "$cloc" == "1" ] && cloc --exclude-list-file=.clocignore "code/" && exit 0
[ "$platform" != "1" ] && [ "$mk" != "1" ] && pf="1" && mk="1" 
[ "$debug" == "1" ] && build_type="$debug_build" 
[ "$release" == "1" ] && build_type="$release_build"
[ "$release" != "1" ] && build_type="$debug_build"

[ "$clean" == "1" ] && echo "deleted /out" && rm -rf "out/"

[ "$build_type" == "$debug_build" ] && echo "[debug build]"
[ "$build_type" == "$release_build" ] && echo "[release build]"

[ ! -d "out" ] && mkdir "out"

[ "$platform" == "1" ] && clang++ $common_flags $build_type code/main.cpp -lSDL3 -I./code/ -o "out/platform" && echo "compiled platform"

[ "$app" == "1" ] && clang++ $common_flags $build_type -fPIC -shared code/saoirse.cpp -o "out/libyk.so" && echo "compiled app"

[ "$run" == "1" ] && ./out/platform