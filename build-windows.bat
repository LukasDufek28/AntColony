@echo off
setlocal
where cmake >nul 2>nul || (echo CMake was not found. Install Visual Studio 2022 Build Tools with C++ support. & pause & exit /b 1)
cmake -S . -B build -A x64 || exit /b 1
cmake --build build --config Release || exit /b 1
echo.
echo Built: build\Release\AntEvolution.exe
pause
