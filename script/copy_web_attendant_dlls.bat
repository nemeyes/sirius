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
MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\configuration
xcopy /Y /S %work_dir%\platform\win32\v140\x86\%build_mode%\bin\configuration\*.*		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\configuration\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\Simd.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_d3d11_video_capturer.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_png_compressor.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_scsp_server.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_unified_server.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_curl_client.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_xml_parser.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_timestamp_generator.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_internal_notifier.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_server_framework.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_attendant_proxy.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\


ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to sirius_web_attendant folder.

:end