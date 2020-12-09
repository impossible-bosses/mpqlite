@echo off

setlocal

set OUTPUT_EXE=main.exe

if "%1"=="" call :help
if "%1"=="help" call :help
if "%1"=="build" call :build
if "%1"=="clean" call :clean
if "%1"=="run" call :run
if "%1"=="test" call :test
if "%1"=="setup" call :setup
if "%1"=="release" call :release
exit /B %ERRORLEVEL%

:help

echo build.bat takes 1 argument: build/clean/run/test/setup/release

exit /B 0

:build

if not exist "build" mkdir build
cd build

set INCLUDE_DIRS=-I ..\libs\StormLib\include -I ..\libs\stb_ds -I ..\libs\stb_sprintf
set LIB_DIRS=-L ..\libs\StormLib\lib\debug
set LIBS=-l user32 -l storm

set ZIG_BUILD_LINE=zig cc -o %OUTPUT_EXE% %INCLUDE_DIRS% %LIB_DIRS% %LIBS% ..\src\main.c
echo %ZIG_BUILD_LINE%
%ZIG_BUILD_LINE%
exit /B %ERRORLEVEL%

rem call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" x64

rem set CL_COMPILER_FLAGS=-nologo -Gm- -GR- -EHa- -EHsc -Z7
rem set CL_WARNING_FLAGS=-WX -W4 -wd4100 -wd4127 -wd4201 -wd4204

rem set CL_LINKER_FLAGS=-incremental:no -opt:ref
rem set CL_LIB_DIRS=-LIBPATH:..\libs\StormLib\lib\debug
rem set CL_LIBS=user32.lib storm.lib

rem cl -MTd -Od %CL_WARNING_FLAGS% %CL_COMPILER_FLAGS% -Fe%OUTPUT_EXE% %INCLUDE_DIRS% ../src/main.c -link %CL_LIB_DIRS% %CL_LIBS%
rem exit /B %ERRORLEVEL%

:clean

echo Cleaning...

rmdir /S /Q build
exit /B 0

:run

.\build\%OUTPUT_EXE%
exit /B %ERRORLEVEL%

:test

echo "test unimplemented"
exit /B 1

:setup

echo "No setup required"
exit /B 0

:release

echo "release unimplemented"
exit /B 1
