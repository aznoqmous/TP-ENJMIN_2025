REM @echo off
.\Tools\premake5.exe --file=premake.lua vs2022
timeout /t 30
if %ERRORLEVEL% NEQ 0 PAUSE