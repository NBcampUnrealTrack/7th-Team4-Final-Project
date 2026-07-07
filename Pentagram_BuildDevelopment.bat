@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

:: ============================
:: 프로젝트, 엔진 위치 등 빌드 관련 설정
:: ============================
set "PROJECT_NAME=Pentagram"
set "ENGINE_PATH=C:\Program Files\Epic Games\UE_5.7\Engine"
set "CONFIG=Development"
set "BUILD_PLATFORM=Win64"

set "PROJECT_DIR=%~dp0"
set "PROJECT_PATH=%PROJECT_DIR%%PROJECT_NAME%.uproject"

set "BATCH_PATH=%ENGINE_PATH%\Build\BatchFiles"
set "UAT_PATH=%BATCH_PATH%\RunUAT.bat"
set "BUILD_PATH=%BATCH_PATH%\Build.bat"

echo ============================
echo 경로 확인
echo PROJECT_PATH=%PROJECT_PATH%
echo BUILD_PATH=%BUILD_PATH%
echo UAT_PATH=%UAT_PATH%
echo ============================

if not exist "%PROJECT_PATH%" (
    echo [오류] uproject 파일을 찾을 수 없음:
    echo "%PROJECT_PATH%"
    pause
    exit /b 1
)

if not exist "%BUILD_PATH%" (
    echo [오류] Build.bat 파일을 찾을 수 없음:
    echo "%BUILD_PATH%"
    pause
    exit /b 1
)

if not exist "%UAT_PATH%" (
    echo [오류] RunUAT.bat 파일을 찾을 수 없음:
    echo "%UAT_PATH%"
    pause
    exit /b 1
)

echo ============================
echo 임시 파일 제거(Binaries, Intermediate)
echo ============================

if exist "%PROJECT_DIR%Binaries" rd /s /q "%PROJECT_DIR%Binaries"
if exist "%PROJECT_DIR%Intermediate" rd /s /q "%PROJECT_DIR%Intermediate"

echo ============================
echo 빌드 시작: %PROJECT_NAME%
echo ============================

echo 에디터 빌드 중...
call "%BUILD_PATH%" "%PROJECT_NAME%Editor" %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex

if errorlevel 1 (
    echo [오류] 에디터 빌드 실패! 코드=!ERRORLEVEL!
    pause
    exit /b !ERRORLEVEL!
)

echo 게임 빌드 중...
call "%BUILD_PATH%" "%PROJECT_NAME%" %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex

if errorlevel 1 (
    echo [오류] 게임 빌드 실패! 코드=!ERRORLEVEL!
    pause
    exit /b !ERRORLEVEL!
)

echo 컨텐츠 쿠킹 및 패키징 중...
call "%UAT_PATH%" BuildCookRun ^
    -project="%PROJECT_PATH%" ^
    -noP4 ^
    -platform=%BUILD_PLATFORM% ^
    -targetplatform=%BUILD_PLATFORM% ^
    -clientconfig=%CONFIG% ^
    -clean ^
    -cook ^
    -allmaps ^
    -map=/Game/Pentagram/Level/L_Intro+/Game/Pentagram/Level/L_Lobby+/Game/Pentagram/Level/01_Abandoned_Mine/L_Abandoned_Mine_01_Quarry ^
    -stage ^
    -pak ^
    -archive ^
    -archivedirectory="%PROJECT_DIR%BuildOutput"

if errorlevel 1 (
    echo [오류] 쿠킹 및 패키징 실패! 코드=!ERRORLEVEL!
    pause
    exit /b !ERRORLEVEL!
)

echo ============================
echo 모든 작업 완료!
echo 출력 위치: "%PROJECT_DIR%BuildOutput"
echo ============================
pause