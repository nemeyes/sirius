@echo off
if not "%VS140DEVENVDIR%" == "" goto BUILD

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86
set VS140DEVENVDIR=TRUE

:BUILD