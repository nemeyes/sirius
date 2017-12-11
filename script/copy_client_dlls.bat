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

MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client
MKDIR %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\configuration
xcopy /Y /S %work_dir%\platform\win32\v140\x86\%build_mode%\bin\configuration\*.*		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\configuration\

IF /i %build_mode%=="release" xcopy /Y %work_dir%\3rdparty\gperftools\v2_6_1\win32\v140\x86\bin\libtcmalloc_minimal.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\3rdparty\simd\v4_0_58_1229\win32\v140\x86\bin\Simd.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\3rdparty\libpng\v1_6_34\win32\v140\x86\bin\zlib1.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\3rdparty\libpng\v1_6_34\win32\v140\x86\bin\libpng16.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_dinput_receiver.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_scsp_client.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_casp_client.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_unified_client.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_png_decompressor.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_ddraw_renderer.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_native_client_framework.dll %work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_client_proxy.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\client\client\


ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to sirius_web_attendant folder.

:end