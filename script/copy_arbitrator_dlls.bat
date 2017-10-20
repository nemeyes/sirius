@ECHO OFF
SETLOCAL

SET build_mode=""
SET work_dir=%1

ECHO %work_dir%

IF "%~2"=="--h" GOTO USAGE1
IF /i "%~2"=="debug" set build_mode="debug"
IF /i "%~2"=="release" set build_mode="release"

IF /i %build_mode%=="" set build_mode="release"
IF /i %work_dir%=="" goto usage1

MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator
MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\configuration
xcopy /Y /S %work_dir%\platform\win32\v140\x86\%build_mode%\bin\configuration\*.*  %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\configuration\



ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to sirius_web_attendant folder.

:end