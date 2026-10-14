@echo off
setlocal
title idocker - Driver Setup Assistant

echo =======================================================
echo   idocker - Automatic iOS USB Filter Driver Setup
echo =======================================================
echo.

:: Check for Administrative Privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] PERINGATAN: Skrip ini membutuhkan Hak Akses Administrator.
    echo [!] Silakan klik kanan file ini lalu pilih "Run as Administrator".
    echo.
    pause
    exit /b 1
)

set DRIVER_DIR=%~dp0drivers\x64

if not exist "%DRIVER_DIR%\install-filter.exe" (
    echo [X] Error: File driver tidak ditemukan di folder %DRIVER_DIR%
    pause
    exit /b 1
)

echo [*] Menginstall service filter libusb0...
"%DRIVER_DIR%\install-filter.exe" install --service

echo [*] Mendaftarkan class filter untuk Apple Mobile Device (0x05AC)...
"%DRIVER_DIR%\install-filter.exe" install --device={36fc9e60-c465-11cf-8056-444553540000}

echo.
echo =======================================================
echo [V] SELESAI! Filter Driver Apple USB berhasil dipasang.
echo [*] Silakan buka kembali aplikasi idocker.exe dan colok iPhone Anda.
echo =======================================================
echo.
pause
