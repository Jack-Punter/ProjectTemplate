@echo off
REM Run Steup the toolchain if CL doesn't exist on the path
call where cl >nul 2>&1 || call .\setup_cl.bat

REM -------------------------- Set Helper Variables --------------------------
set me="%~dp0"
cd %me%
set root=%cd%
set build_dir=%root%\build
set src_dir=%root%\src
set include_dir=%root%\include

set CommonCompilerFlags=-MT -nologo -Gm- -Zi -FC -GR- -EHa -W3 -fp:fast -fp:except- -arch:AVX2 -std:c11
REM set CommonCompilerFlags=%CommonCompilerFlags% -diagnostics:caret
set CommonCompilerFlags=%CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS

set DebugCompilerFlags=-Od
set ReleaseCompilerFlags=-O2

REM 4005: macro redefinition, comes from spall with _CRT_SECURE_NO_WARNINGS
set AutoTracingCompilerFlags=-Gh -GH -DAUTO_TRACING -wd4005

set CommonLinkerFlags=-SUBSYSTEM:windows User32.lib gdi32.lib Winmm.lib

REM -------------------------- Check Build Arguments --------------------------
set build_mode=debug
set auto_tracing=off

for %%v in (%*) do call :ParseArg %%v

if NOT %ERRORLEVEL%==0 (
  exit /B %ERRORLEVEL%
)

echo ============================
echo Build mode:   %build_mode%   
echo Auto tracing: %auto_tracing% 
echo ============================

REM -------------------------- Set Build Options --------------------------
set BuildFlags=%CommonCompilerFlags%

if "%build_mode%"=="release" (
  set BuildFlags=%BuildFlags% %ReleaseCompilerFlags%
) else (
  set BuildFlags=%BuildFlags% %DebugCompilerFlags%
)

if "%auto_tracing%"=="on" (
  set BuildFlags=%BuildFlags% %AutoTracingCompilerFlags%
)

REM -------------------------- Build the project --------------------------
IF NOT EXIST %build_dir% mkdir %build_dir%

REM Clear the build directory 
REM echo y | del %build_dir% > NUL 2> NUL

pushd %build_dir%
  del *.pdb > NUL 2> NUL

  echo WAITING FOR PDB > lock.tmp

  cl %BuildFlags%  %src_dir%\app_main.cpp -I%include_dir% -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:app_update
  del lock.tmp

  cl %BuildFlags% %src_dir%\win32_platform.cpp -I%include_dir% /Feapp_core.exe /link -incremental:no %CommonLinkerFlags%
popd

EXIT /B %ERRORLEVEL%

REM -------------------------- Argument Handling --------------------------
:ParseArg
  set release_flag=release
  set tracing_flag=tracing
  
  if "%~1"=="%release_flag%" (
    set build_mode=release
  ) else if "%~1"=="%tracing_flag%" (
    set auto_tracing=on
  ) else (
    echo Invalid Argument "%~1"
    EXIT /B 1
  )
EXIT /B 0
