@echo off

setlocal

set OUTPUT_EXE=mpqlite.exe

set DEFINES=-DAPP_WIN32
set INCLUDE_DIRS=-I ..\libs\StormLib\include -I ..\libs\stb_sprintf
set LIB_DIRS=-L ..\libs\StormLib\lib\debug
set LIBS=-l user32 -l storm

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

set ZIG_BUILD_LINE=zig cc ..\src\main.c %DEFINES% -o %OUTPUT_EXE% %INCLUDE_DIRS% %LIB_DIRS% %LIBS%
echo %ZIG_BUILD_LINE%
%ZIG_BUILD_LINE%
exit /B %ERRORLEVEL%

:clean

echo Cleaning...

rmdir /S /Q build
exit /B 0

:run

.\build\%OUTPUT_EXE%
exit /B %ERRORLEVEL%

:test

if not exist "build" mkdir build
cd build

set ZIG_BUILD_LINE=zig cc ..\src\test.c %DEFINES% -o test.exe %INCLUDE_DIRS% %LIB_DIRS% %LIBS%
echo %ZIG_BUILD_LINE%
%ZIG_BUILD_LINE%
if %ERRORLEVEL% neq 0 (
	exit /B %ERRORLEVEL%
)

copy ..\test\* .
.\test.exe
exit /B %ERRORLEVEL%

:setup

echo "No setup required"
exit /B 0

:release

echo "release unimplemented"
exit /B 1
