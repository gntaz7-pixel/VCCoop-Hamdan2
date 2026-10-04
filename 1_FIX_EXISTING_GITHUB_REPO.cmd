@echo off
setlocal EnableExtensions
rem HOW TO USE: Clone your existing GitHub repository in GitHub Desktop.
rem Extract this source archive somewhere outside the clone.
rem Drag the CLONED repository folder onto THIS CMD file.
if "%~1"=="" (
  echo ERROR: Drag your CLONED VCCoop-Hamdan1 repository folder onto this CMD file.
  echo First clone it in GitHub Desktop: File ^> Clone repository.
  pause
  exit /b 1
)
set "TARGET=%~f1"
if not exist "%TARGET%\.git" (
  echo ERROR: The dropped folder is not a Git repository clone: "%TARGET%"
  echo Do not drag the .zip file; clone the repository using GitHub Desktop first.
  pause
  exit /b 2
)
if not exist "%~dp0src\dinput8.def" (
  echo ERROR: The extracted source folder is incomplete.
  pause
  exit /b 3
)
echo Copying patched source and GitHub Actions into the ROOT of:
echo   "%TARGET%"
for %%D in (src tests launcher rt dist .github) do (
  robocopy "%~dp0%%D" "%TARGET%\%%D" /E /R:1 /W:1 /NFL /NDL /NJH /NJS >nul
  if errorlevel 8 (
    echo ERROR: Copying %%D failed.
    pause
    exit /b 4
  )
)
for %%F in (build_NPC_fix.cmd .gitignore README_HAMDAN_AR.txt README_INSTALL_AR.txt HAMDAN-vccoop-options.ini) do (
  copy /Y "%~dp0%%F" "%TARGET%\%%F" >nul
  if errorlevel 1 (
    echo ERROR: Copying %%F failed.
    pause
    exit /b 5
  )
)
if not exist "%TARGET%\.github\workflows\build-hamdan.yml" (
  echo ERROR: The GitHub Actions workflow was not copied.
  pause
  exit /b 6
)
echo.
echo SUCCESS: Source and .github workflow now exist at repository ROOT.
echo No game DLL was replaced, and the old nested folder was not deleted.
echo NEXT: Open GitHub Desktop, COMMIT changes and PUSH ORIGIN.
echo THEN: GitHub Actions ^> Build VCCoop Hamdan DLL (Win32) ^> Run workflow.
pause
exit /b 0
