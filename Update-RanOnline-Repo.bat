@echo off
setlocal EnableExtensions EnableDelayedExpansion

title RAN Online - GitHub Update v2

set "REPO=D:\FILES\project\modernization RanOnline"
set "REMOTE=https://github.com/mikkkoyy/RanOnline-Source.git"
set "BRANCH=main"
set "COMMIT_MSG=Update RAN Online source"
set "GITIGNORE=%REPO%\.gitignore"

echo.
echo ========================================
echo      RAN ONLINE GITHUB UPDATE v2
echo ========================================
echo.
echo Repository:
echo %REPO%
echo Remote:
echo %REMOTE%
echo Branch:
echo %BRANCH%
echo.
echo [SAFE MODE]
echo - No force push
echo - No history reset
echo - No automatic pull/merge
echo - Remote-ahead history is blocked
echo - Diverged history is blocked
echo.

REM ============================================================
REM 1. CHECK GIT
REM ============================================================

where git >nul 2>&1
if errorlevel 1 goto :error_git

for /f "delims=" %%A in ('git --version 2^>nul') do set "GIT_VERSION=%%A"

echo Git:
echo !GIT_VERSION!
echo.

REM ============================================================
REM 2. CHECK REPOSITORY
REM ============================================================

if not exist "%REPO%\" goto :error_repo

cd /d "%REPO%" || goto :error_repo

echo [1/7] Repository check...

if not exist ".git\" (
    echo.
    echo [INFO] Git repository not initialized.
    echo Initializing repository...
    git init

    if errorlevel 1 goto :error_init

    echo.
    echo Git repository initialized.
)

REM ============================================================
REM 3. ENSURE BRANCH
REM ============================================================

echo.
echo [2/7] Checking branch...

git branch -M "%BRANCH%"
if errorlevel 1 goto :error_branch

REM ============================================================
REM 4. CONFIGURE REMOTE
REM ============================================================

echo.
echo [3/7] Checking GitHub remote...

git remote get-url origin >nul 2>&1

if errorlevel 1 (
    echo Adding GitHub remote...
    git remote add origin "%REMOTE%"

    if errorlevel 1 goto :error_remote
) else (
    for /f "delims=" %%A in ('git remote get-url origin 2^>nul') do set "CURRENT_REMOTE=%%A"

    if /I not "!CURRENT_REMOTE!"=="%REMOTE%" (
        echo Existing remote:
        echo !CURRENT_REMOTE!
        echo.
        echo Updating remote to:
        echo %REMOTE%

        git remote set-url origin "%REMOTE%"

        if errorlevel 1 goto :error_remote
    )
)

echo Remote:
git remote -v

REM ============================================================
REM 5. CREATE / VERIFY .GITIGNORE
REM ============================================================

echo.
echo [4/7] Checking .gitignore...

if not exist "%GITIGNORE%" (
    echo Creating .gitignore...

    >"%GITIGNORE%" (
        echo # ============================================================
        echo # RAN Online / Visual Studio Git Ignore
        echo # ============================================================
        echo.
        echo # Visual Studio
        echo .vs/
        echo *.user
        echo *.suo
        echo *.userosscache
        echo *.sdf
        echo *.VC.db
        echo *.VC.VC.opendb
        echo.
        echo # Build output
        echo [Bb]in/
        echo [Oo]bj/
        echo Debug/
        echo Release/
        echo x64/
        echo x86/
        echo build/
        echo out/
        echo.
        echo # Compiled files
        echo *.obj
        echo *.pdb
        echo *.ilk
        echo *.idb
        echo *.tlog
        echo *.lastbuildstate
        echo.
        echo # Binaries
        echo *.dll
        echo *.exe
        echo *.lib
        echo *.exp
        echo.
        echo # CMake generated files
        echo CMakeFiles/
        echo CMakeCache.txt
        echo cmake_install.cmake
        echo CTestTestfile.cmake
        echo install_manifest.txt
        echo.
        echo # Temporary files
        echo *.tmp
        echo *.temp
        echo *.bak
        echo *.orig
        echo.
        echo # Crash/debug dumps
        echo *.dmp
        echo *.stackdump
    )

    if errorlevel 1 goto :error_gitignore

    echo .gitignore created.
) else (
    echo .gitignore already exists.
)

REM ============================================================
REM 6. CHECK GITHUB BEFORE CHANGING ANYTHING
REM ============================================================

echo.
echo [5/7] Checking GitHub...

git ls-remote --heads origin "%BRANCH%"

if errorlevel 1 goto :error_github

echo.
echo GitHub repository is accessible.

REM ============================================================
REM 7. FETCH REMOTE
REM ============================================================

echo.
echo [6/7] Fetching GitHub...

git fetch origin "%BRANCH%"

if errorlevel 1 goto :error_fetch

REM ============================================================
REM 8. HISTORY SAFETY CHECK
REM ============================================================

echo.
echo Checking local and remote history...

set "LOCAL_HEAD="
set "REMOTE_HEAD="

for /f "delims=" %%A in ('git rev-parse --verify HEAD 2^>nul') do set "LOCAL_HEAD=%%A"

for /f "delims=" %%A in ('git rev-parse --verify origin/%BRANCH% 2^>nul') do set "REMOTE_HEAD=%%A"

REM ------------------------------------------------------------
REM No local commit yet
REM ------------------------------------------------------------

if not defined LOCAL_HEAD (
    echo.
    echo [INFO] No local commit exists yet.
    goto :stage
)

REM ------------------------------------------------------------
REM Remote does not have a commit
REM ------------------------------------------------------------

if not defined REMOTE_HEAD (
    echo.
    echo [INFO] GitHub has no existing commit on %BRANCH%.
    goto :stage
)

echo.
echo Local HEAD:
echo %LOCAL_HEAD%
echo.
echo GitHub HEAD:
echo %REMOTE_HEAD%

REM ------------------------------------------------------------
REM Remote is already contained in local history
REM Safe to continue.
REM ------------------------------------------------------------

git merge-base --is-ancestor "%REMOTE_HEAD%" "%LOCAL_HEAD%" >nul 2>&1

if not errorlevel 1 (
    echo.
    echo [OK] GitHub history is already contained in local history.
    goto :stage
)

REM ------------------------------------------------------------
REM Local is behind GitHub
REM ------------------------------------------------------------

git merge-base --is-ancestor "%LOCAL_HEAD%" "%REMOTE_HEAD%" >nul 2>&1

if not errorlevel 1 goto :error_behind

REM ------------------------------------------------------------
REM Histories diverged
REM ------------------------------------------------------------

goto :error_diverged


:stage

REM ============================================================
REM 9. STAGE SOURCE
REM ============================================================

echo.
echo [7/7] Staging source changes...

git add -A

if errorlevel 1 goto :error_stage

echo.
echo Checking staged changes...

git status --short

echo.

REM ============================================================
REM 10. CHECK FOR CHANGES
REM ============================================================

git diff --cached --quiet

if not errorlevel 1 (
    echo.
    echo ========================================
    echo          NO CHANGES TO COMMIT
    echo ========================================
    echo.
    echo Source and GitHub are already synchronized.
    echo.
    goto :success_no_changes
)

REM ============================================================
REM 11. SHOW STAGED SUMMARY
REM ============================================================

echo.
echo ========================================
echo          STAGED CHANGES
echo ========================================
echo.

git diff --cached --stat

echo.
echo Files that will be committed:
echo.

git diff --cached --name-status

echo.

REM ============================================================
REM 12. COMMIT
REM ============================================================

echo ========================================
echo             COMMIT
echo ========================================
echo.

echo Commit message:
echo %COMMIT_MSG%
echo.

git commit -m "%COMMIT_MSG%"

if errorlevel 1 goto :error_commit

REM ============================================================
REM 13. FINAL PUSH SAFETY CHECK
REM ============================================================

echo.
echo ========================================
echo          PUSH SAFETY CHECK
echo ========================================
echo.

echo Normal push only.
echo Force push is NOT permitted.
echo.

REM Explicitly use normal push.
REM Do NOT add --force, --force-with-lease, or any reset command.

git push -u origin "%BRANCH%"

if errorlevel 1 goto :error_push

REM ============================================================
REM SUCCESS
REM ============================================================

:success

echo.
echo ========================================
echo          UPDATE COMPLETE
echo ========================================
echo.
echo RAN Online source was successfully pushed.
echo.
echo Repository:
echo %REPO%
echo.
echo GitHub:
echo %REMOTE%
echo.
echo Branch:
echo %BRANCH%
echo.
echo Build output remains outside the repository:
echo D:\FILES\project\RanOnline-Build
echo.
echo No force push was performed.
echo.

git status --short

echo.
pause
exit /b 0


:success_no_changes

echo.
echo Build output remains outside the repository:
echo D:\FILES\project\RanOnline-Build
echo.
echo No force push was performed.
echo.
pause
exit /b 0


REM ============================================================
REM ERROR HANDLERS
REM ============================================================

:error_git

echo.
echo ========================================
echo [ERROR] Git is not available.
echo ========================================
echo.
echo Git was not found in PATH.
echo Install Git or add it to PATH.
echo.
pause
exit /b 1


:error_repo

echo.
echo ========================================
echo [ERROR] Repository folder not found.
echo ========================================
echo.
echo Expected:
echo %REPO%
echo.
pause
exit /b 1


:error_init

echo.
echo ========================================
echo [ERROR] Git initialization failed.
echo ========================================
echo.
echo Git output is shown above.
echo.
pause
exit /b 1


:error_branch

echo.
echo ========================================
echo [ERROR] Could not set branch.
echo ========================================
echo.
echo Git output is shown above.
echo.
pause
exit /b 1


:error_remote

echo.
echo ========================================
echo [ERROR] Could not configure GitHub remote.
echo ========================================
echo.
echo Expected remote:
echo %REMOTE%
echo.
echo Git output is shown above.
echo.
pause
exit /b 1


:error_gitignore

echo.
echo ========================================
echo [ERROR] Could not create .gitignore.
echo ========================================
echo.
echo Git output is shown above.
echo.
pause
exit /b 1


:error_github

echo.
echo ========================================
echo [ERROR] Cannot access GitHub.
echo ========================================
echo.
echo Remote:
echo %REMOTE%
echo.
echo Check:
echo - Internet connection
echo - GitHub availability
echo - GitHub authentication
echo - Repository permissions
echo.
echo Git output is shown above.
echo.
pause
exit /b 1


:error_fetch

echo.
echo ========================================
echo [ERROR] GitHub fetch failed.
echo ========================================
echo.
echo The remote repository could not be fetched.
echo.
echo Git output is shown above.
echo.
echo NO push was performed.
echo.
pause
exit /b 1


:error_behind

echo.
echo ========================================
echo [BLOCKED] LOCAL REPOSITORY IS BEHIND
echo ========================================
echo.
echo GitHub contains commits that are not present locally.
echo.
echo Local:
echo %LOCAL_HEAD%
echo.
echo GitHub:
echo %REMOTE_HEAD%
echo.
echo The script will NOT automatically pull or merge.
echo The script will NOT force-push.
echo.
echo Reconcile the histories manually first.
echo.
pause
exit /b 2


:error_diverged

echo.
echo ========================================
echo [BLOCKED] HISTORIES HAVE DIVERGED
echo ========================================
echo.
echo Local and GitHub contain different histories.
echo.
echo Local:
echo %LOCAL_HEAD%
echo.
echo GitHub:
echo %REMOTE_HEAD%
echo.
echo The script will NOT:
echo - reset history
echo - pull automatically
echo - merge automatically
echo - force push
echo.
echo Resolve the histories manually before running this script again.
echo.
pause
exit /b 2


:error_stage

echo.
echo ========================================
echo [ERROR] Failed to stage files.
echo ========================================
echo.
echo Git output is shown above.
echo.
echo No push was performed.
echo.
pause
exit /b 1


:error_commit

echo.
echo ========================================
echo [ERROR] Commit failed.
echo ========================================
echo.
echo Git output is shown above.
echo.
echo No push was performed.
echo.
pause
exit /b 1


:error_push

echo.
echo ========================================
echo [ERROR] Push failed.
echo ========================================
echo.
echo GitHub rejected the normal push.
echo.
echo IMPORTANT:
echo No force push was attempted.
echo No history was reset.
echo.
echo Check the Git output above.
echo.
pause
exit /b 1
