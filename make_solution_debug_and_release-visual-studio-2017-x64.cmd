@echo off
set CMAKE_EXECUTABLE="cmake.exe"
set SOURCE_DIR=%cd%
set BUILD_DIR=%cd%\cmake-vs-x64-build

if exist "%BUILD_DIR%" rd /s /q %BUILD_DIR%

mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"
%CMAKE_EXECUTABLE% -G "Visual Studio 15 2017 Win64" -DPOCO_ROOT_DIR="D:/devel-soft/Poco64-devel" -DGTEST_ROOT_DIR="D:/devel-soft/gtest64" -DPROTOBUF_ROOT_DIR="D:/devel-soft/protobuf64" -DCMAKE_INSTALL_PREFIX="D:/devel-soft/upmq" "%SOURCE_DIR%"
cd "%SOURCE_DIR%"

%CMAKE_EXECUTABLE% --build "%BUILD_DIR%" --config Debug
%CMAKE_EXECUTABLE% --build "%BUILD_DIR%" --target install