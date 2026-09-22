@echo off
setlocal EnableExtensions

title RAN Online - GitHub Update

set "REPO=D:\FILES\project\modernization RanOnline"
set "REMOTE=https://github.com/mikkkoyy/RanOnline-Source.git"
set "BRANCH=main"
set "COMMIT_MSG=Update RAN Online source"

echo.
echo ========================================
echo       RAN ONLINE GITHUB UPDATE
echo ========================================
echo.

where git >nul 2>&1
if errorlevel 1 goto :error_git

if not exist "%REPO%\" goto :error_repo

cd /d "%REPO%" || goto :error_repo

if not exist ".git\" (
    git init >nul 2>&1 || goto :error_git
)

git branch -M "%BRANCH%" >nul 2>&1

git remote get-url origin >nul 2>&1
if errorlevel 1 (
    git remote add origin "%REMOTE%" >nul 2>&1
) else (
    git remote set-url origin "%REMOTE%" >nul 2>&1
)

if errorlevel 1 goto :error_remote

echo [1/5] Checking GitHub...
git ls-remote --heads origin "%BRANCH%" >nul 2>&1
if errorlevel 1 goto :error_github

echo [2/5] Fetching...
git fetch origin "%BRANCH%" >nul 2>&1
if errorlevel 1 goto :error_fetch

for /f "delims=" %%A in ('git rev-parse --verify HEAD 2^>nul') do set "LOCAL_HEAD=%%A"
for /f "delims=" %%A in ('git rev-parse --verify origin/%BRANCH% 2^>nul') do set "REMOTE_HEAD=%%A"

if defined LOCAL_HEAD if defined REMOTE_HEAD (
    git merge-base --is-ancestor "%REMOTE_HEAD%" "%LOCAL_HEAD%" >nul 2>&1

    if errorlevel 1 (
        git merge-base --is-ancestor "%LOCAL_HEAD%" "%REMOTE_HEAD%" >nul 2>&1

        if not errorlevel 1 goto :error_behind

        goto :error_diverged
    )
)

echo [3/5] Staging...
git add -A
if errorlevel 1 goto :error_stage

git diff --cached --quiet
if errorlevel 1 (
    echo [4/5] Committing...
    git commit -m "%COMMIT_MSG%" >nul 2>&1
    if errorlevel 1 goto :error_commit
) else (
    echo [4/5] No changes to commit.
)

echo [5/5] Pushing...
git push -u origin "%BRANCH%" >nul 2>&1
if errorlevel 1 goto :error_push

echo.
echo ========================================
echo             UPDATE COMPLETE
echo ========================================
echo.
echo Repository updated successfully.
echo.
pause
exit /b 0


:error_git
echo.
echo [ERROR] Git is not available.
pause
exit /b 1

:error_repo
echo.
echo [ERROR] Repository folder not found:
echo %REPO%
pause
exit /b 1

:error_remote
echo.
echo [ERROR] Could not configure GitHub remote.
pause
exit /b 1

:error_github
echo.
echo [ERROR] Cannot access GitHub repository.
pause
exit /b 1

:error_fetch
echo.
echo [ERROR] GitHub fetch failed.
pause
exit /b 1

:error_behind
echo.
echo [BLOCKED] GitHub has commits not present locally.
echo Pull/reconcile the changes before updating.
pause
exit /b 2

:error_diverged
echo.
echo [BLOCKED] Local and GitHub histories have diverged.
echo Resolve the histories manually. No force-push was performed.
pause
exit /b 2

:error_stage
echo.
echo [ERROR] Failed to stage files.
pause
exit /b 1

:error_commit
echo.
echo [ERROR] Commit failed.
pause
exit /b 1

:error_push
echo.
echo [ERROR] Push failed.
echo Check GitHub authentication and permissions.
pause
exit /b 1
