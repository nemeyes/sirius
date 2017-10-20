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

MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web
xcopy /Y /S %work_dir%\3rdparty\cef\3.2987.1600.g9ea5b3b\win32\v140\x86\resources\*.*  %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y /S %work_dir%\3rdparty\cef\3.2987.1600.g9ea5b3b\win32\v140\x86\%build_mode%\*.*  %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to cef folder.

:end