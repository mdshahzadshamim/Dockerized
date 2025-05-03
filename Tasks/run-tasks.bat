@echo off
echo 📦 Starting the database container...
docker compose up -d db

echo ⏳ Waiting for the database to be ready...
:wait_for_db
FOR /F "tokens=*" %%i IN ('docker compose exec db pg_isready 2^>NUL') DO (
    echo %%i | findstr /C:"accepting connections" >NUL
    IF NOT ERRORLEVEL 1 (
        echo ✅ Database is ready.
        goto db_ready
    )
)
timeout /t 1 > NUL
goto wait_for_db

:db_ready
echo 🚀 Starting the Tasks app...
docker compose run --rm --entrypoint ./build/Tasks app

set /p STOP=🛑 Do you want to stop the database container now? [y/N] 
if /I "%STOP%"=="y" (
    echo 🧹 Stopping and cleaning up...
    docker compose down
) else (
    echo ℹ Database is still running. You can stop it manually with: docker compose down
)