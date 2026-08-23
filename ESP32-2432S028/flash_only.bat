@echo off
setlocal
cd /d "%~dp0"

set IDF_TOOLS_PATH=D:\Espressif
set IDF_PATH=D:\esp\v6.0.2\esp-idf
set IDF_PYTHON_ENV_PATH=D:\Espressif\tools\python\v6.0.2\venv
set PORT=COM3
if not "%~1"=="" set PORT=%~1

if not exist "%IDF_TOOLS_PATH%\espidf.constraints.v6.0.txt" (
  if exist "%IDF_TOOLS_PATH%\tools\espidf.constraints.v6.0.txt" (
    copy /Y "%IDF_TOOLS_PATH%\tools\espidf.constraints.v6.0.txt" "%IDF_TOOLS_PATH%\espidf.constraints.v6.0.txt" >nul
  )
)

echo.
echo Port: %PORT%  ^(already built^)
echo Hold BOOT, tap RESET, keep holding BOOT, then press any key
pause >nul

call "%IDF_PATH%\export.bat"
if errorlevel 1 exit /b 1

cd /d "%~dp0build"
python -m esptool --chip esp32 -p %PORT% -b 115200 --before no-reset --after hard-reset write-flash --flash-mode dio --flash-freq 40m --flash-size 4MB 0x1000 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x10000 ESP32-2432S028.bin
set ERR=%ERRORLEVEL%
cd /d "%~dp0"
if %ERR% neq 0 exit /b %ERR%
echo Flash OK. Release BOOT.
endlocal
