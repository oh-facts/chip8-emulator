@echo off
setlocal
cd /D "%~dp0"

:: Usage
:: ==============
:: ./build platform    : Builds platform
:: ./build app         : Builds app
:: ./build clean       : Deletes out/ folder

set debug_build="-O0 -g"
set release_build="-O3"
set build_type=""
for %%a in (%*) do set "%%a=1"

set common_c_flags="-std=c99 -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment -g"

set common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment -g"

if "%cloc%" == "1" cloc --exclude-list-file=.clocignore .\code\
if "%clean%" == "1" rmdir /s /q "out" && del "code\meta.h"
if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

if not exist out mkdir out 

if "%metacr%" == "1" clang "%common_c_flags%" code/cpp.c -o out/meta.exe && .\out\meta.exe > code\meta.h && echo [generating code]

if "%platform%" == "1" echo [platform] && clang++ "%common_flags%" "%build_type%" -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lopengl32 code/main.cpp -o out/platform.exe

if %errorlevel% neq 0 echo platform compilation failed && exit /b

if "%app%" == "1" echo [app] && clang++ "%common_flags%" "%build_type%" code/saoirse.cpp -shared -o out/yk.dll

if %errorlevel% neq 0 echo app compilation failed && exit /b

if "%run%" == "1" .\out\platform.exe && echo [run]