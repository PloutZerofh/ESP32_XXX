@echo off
REM Activate local ESP-IDF 6.0 toolchain for this machine
set IDF_TOOLS_PATH=D:\Espressif
set IDF_PATH=D:\esp\v6.0.2\esp-idf
set IDF_PYTHON_ENV_PATH=D:\Espressif\tools\python\v6.0.2\venv
if not exist "%IDF_TOOLS_PATH%\espidf.constraints.v6.0.txt" (
  if exist "%IDF_TOOLS_PATH%\tools\espidf.constraints.v6.0.txt" (
    copy /Y "%IDF_TOOLS_PATH%\tools\espidf.constraints.v6.0.txt" "%IDF_TOOLS_PATH%\espidf.constraints.v6.0.txt" >nul
  )
)
call "%IDF_PATH%\export.bat"
