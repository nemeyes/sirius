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
IF /i %build_mode%=="release" xcopy /Y %work_dir%\3rdparty\gperftools\v2_6_1\win32\v140\x86\bin\libtcmalloc_minimal.dll	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\Microsoft.DTfW.DHL.manifest	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\vld_x86.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\dbghelp.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\vcruntime140d.dll		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\ucrtbased.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\msvcp140d.dll			%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.dll				%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_log4cplus_logger.pdb	%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_performance_monitor.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_performance_monitor.pdb		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_d3d11_device_stat.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_d3d11_device_stat.pdb		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_arbitrator_launcher.exe					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_arbitrator_launcher.pdb		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\

xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_arbitrator_proxy.dll					%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\
IF /i %build_mode%=="debug" xcopy /Y %work_dir%\platform\win32\v140\x86\%build_mode%\bin\sirius_arbitrator_proxy.pdb		%work_dir%\platform\win32\v140\x86\%build_mode%\bin\apps\arbitrator\




ECHO copy complited.
goto end

:: --------------------
:USAGE1
:: --------------------
ECHO usage: [DEBUG^|RELEASE] work_dir
ECHO.
ECHO copy necessary files to sirius_web_attendant folder.

:end