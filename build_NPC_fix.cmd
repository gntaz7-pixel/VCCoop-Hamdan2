@echo off
rem Builds ONLY the patched VCCoop 32-bit dinput8.dll using Visual Studio 2022.
rem Finds Visual Studio under BOTH Program Files roots.
setlocal EnableExtensions
cd /d "%~dp0"
set "VCVARS="
for %%E in (BuildTools Community Professional Enterprise) do (
 if exist "%ProgramFiles%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars32.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars32.bat"
 if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars32.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars32.bat"
)
if not defined VCVARS (
 echo ERROR: Visual Studio 2022 C++ Build Tools x86 not found.
 echo Install 'Desktop development with C++' or use the included GitHub Actions workflow.
 if not defined CI pause
 exit /b 1
)
call "%VCVARS%" >nul
if errorlevel 1 goto :failed
where cl >nul 2>&1
if errorlevel 1 goto :failed
if not exist build mkdir build
cl /nologo /O2 /MT /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /Fobuild\ src\*.cpp launcher\model3d.cpp ^
 /LD /Febuild\dinput8.dll /link /MAP:build\dinput8.map /DEF:src\dinput8.def user32.lib kernel32.lib advapi32.lib ws2_32.lib winmm.lib
if errorlevel 1 goto :failed
if not exist build\dinput8.dll goto :failed
echo SUCCESS: %CD%\build\dinput8.dll
if not defined CI pause
exit /b 0
:failed
echo BUILD FAILED: inspect compiler errors above.
if not defined CI pause
exit /b 1
