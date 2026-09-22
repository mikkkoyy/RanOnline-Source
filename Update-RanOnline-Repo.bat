@echo off
setlocal EnableExtensions

title RAN Online - Update GitHub Repository

REM ============================================================
REM RAN Online Repository Updater
REM Local source:
REM D:\FILES\project\modernization RanOnline
REM GitHub:
REM https://github.com/mikkkoyy/RanOnline-Source.git
REM ============================================================

set "REPO=D:\FILES\project\modernization RanOnline"
set "REMOTE=https://github.com/mikkkoyy/RanOnline-Source.git"
set "BRANCH=main"

echo.
echo ============================================================
echo   RAN ONLINE REPOSITORY UPDATE
echo ============================================================
echo.
echo Local : %REPO%
echo Remote: %REMOTE%
echo Branch: %BRANCH%
echo.

REM ------------------------------------------------------------
REM Check Git
REM ------------------------------------------------------------
where git >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git was not found in PATH.
    echo Install Git or add Git to PATH.
    echo.
    pause
    exit /b 1
)

REM ------------------------------------------------------------
REM Check repository directory
REM ------------------------------------------------------------
if not exist "%REPO%\" (
    echo [ERROR] Repository directory does not exist:
    echo %REPO%
    echo.
    pause
    exit /b 1
)

cd /d "%REPO%"

REM ------------------------------------------------------------
REM Initialize Git if this is not already a repository
REM ------------------------------------------------------------
if not exist ".git\" (
    echo [INFO] Git repository not initialized.
    echo [INFO] Initializing...
    echo.

    git init

    if errorlevel 1 (
        echo.
        echo [ERROR] git init failed.
        pause
        exit /b 1
    )
)

REM ------------------------------------------------------------
REM Ensure branch is main
REM ------------------------------------------------------------
git branch -M %BRANCH%

REM ------------------------------------------------------------
REM Configure GitHub remote
REM ------------------------------------------------------------
git remote get-url origin >nul 2>&1

if errorlevel 1 (
    echo [INFO] Adding GitHub remote...
    git remote add origin "%REMOTE%"

    if errorlevel 1 (
        echo.
        echo [ERROR] Could not add GitHub remote.
        pause
        exit /b 1
    )
) else (
    echo [INFO] Updating GitHub remote...
    git remote set-url origin "%REMOTE%"

    if errorlevel 1 (
        echo.
        echo [ERROR] Could not update GitHub remote.
        pause
        exit /b 1
    )
)

echo.
echo ============================================================
echo   CURRENT STATUS
echo ============================================================
echo.

git status --short

echo.
echo ============================================================
echo   STAGING FILES
echo ============================================================
echo.

git add .

if errorlevel 1 (
    echo.
    echo [ERROR] git add failed.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo   FILES READY TO COMMIT
echo ============================================================
echo.

git status --short

echo.
echo ============================================================
echo   COMMIT
echo ============================================================
echo.

set "COMMIT_MSG=Update RAN Online source"

set /p "CUSTOM_MSG=Commit message [%COMMIT_MSG%]: "

if not "%CUSTOM_MSG%"=="" (
    set "COMMIT_MSG=%CUSTOM_MSG%"
)

git diff --cached --quiet

if errorlevel 1 (
    git commit -m "%COMMIT_MSG%"

    if errorlevel 1 (
        echo.
        echo [ERROR] Commit failed.
        pause
        exit /b 1
    )
) else (
    echo [INFO] No file changes to commit.
)

echo.
echo ============================================================
echo   PUSH TO GITHUB
echo ============================================================
echo.

git push -u origin %BRANCH%

if errorlevel 1 (
    echo.
    echo [ERROR] Push failed.
    echo.
    echo Possible causes:
    echo   - GitHub authentication is not configured
    echo   - Repository permissions are missing
    echo   - Network problem
    echo   - Remote repository contains commits not present locally
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo   UPDATE COMPLETE
echo ============================================================
echo.
echo Repository:
echo %REMOTE%
echo.
echo Local source:
echo %REPO%
echo.
echo Branch:
echo %BRANCH%
echo.
echo GitHub has been updated successfully.
echo.

git status

echo.
pause
exit /b 0
