@echo off
echo ===============================
echo Cleaning Unreal Engine Caches
echo ===============================

:: 현재 디렉토리를 프로젝트 루트로 사용
set "PROJECT_DIR=%CD%"
echo Project Directory: %PROJECT_DIR%
echo.

:: 삭제
echo Removing Intermediate...
for /d /r . %%d in (Intermediate) do @if exist "%%d" rd /s /q "%%d"

echo Removing Binaries...
for /d /r . %%d in (Binaries) do @if exist "%%d" rd /s /q "%%d"

echo Removing Saved...
for /d /r . %%d in (Saved) do @if exist "%%d" rd /s /q "%%d"

echo Removing .vs...
for /d /r . %%d in (.vs) do @if exist "%%d" rd /s /q "%%d"

echo Removing DerivedDataCache (if exists)...
for /d /r . %%d in (DerivedDataCache) do @if exist "%%d" rd /s /q "%%d"

echo Removing .sln files...
del /q "%PROJECT_DIR%\*.sln"
rmdir .idea

echo ===============================
echo All cache folders removed.
echo.