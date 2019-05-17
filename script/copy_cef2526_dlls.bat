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
xcopy /Y /S %work_dir%\3rdparty\cef\3.2526.1373.gb660893\win32\v140\x86\resources\*.*					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y /S %work_dir%\3rdparty\cef\3.2526.1373.gb660893\win32\v140\x86\%build_mode%\*.*				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y /S %work_dir%\platform\win32\v140\x86\%build_mode%\bin\configuration\*.*					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\configuration\

IF /i %build_mode%=="release" xcopy /Y %work_dir%\3rdparty\gperftools\v2_6_1\win32\v140\x86\bin\libtcmalloc_minimal.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\3rdparty\libpng\v1_6_34\win32\v140\x86\bin\zlib1.dll						%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\3rdparty\libpng\v1_6_34\win32\v140\x86\bin\libpng16.dll						%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\Microsoft.DTfW.DHL.manifest	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\vld_x86.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\dbghelp.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\vcruntime140d.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\ucrtbased.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\msvcp140d.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_cpu_video_capturer.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_cpu_video_capturer.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_cpu_video_capturer.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_localcache_client.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_localcache_client.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_localcache_client.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_png_compressor.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_png_compressor.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_png_compressor.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_webp_compressor.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_webp_compressor.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_partial_webp_compressor.ipdb			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_scsp_server.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_scsp_server.pdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_scsp_server.ipdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_unified_server.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_unified_server.pdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_unified_server.ipdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_curl_client.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_curl_client.pdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_curl_client.ipdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_xml_parser.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_xml_parser.pdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_xml_parser.ipdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_timestamp_generator.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_timestamp_generator.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_timestamp_generator.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_internal_notifier.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_internal_notifier.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_internal_notifier.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_server_framework.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_server_framework.pdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_server_framework.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_attendant_proxy.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_attendant_proxy.pdb					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_attendant_proxy.ipdb				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_attendant.bat					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\
xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_web_attendant_gpu.bat				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\attendants\web\



ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to sirius_web_attendant folder.

:end