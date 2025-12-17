@echo off
echo Building Desktop Communication Suite...
g++ -std=c++17 -municode -mwindows src/*.cpp -o ChatSuite.exe -lws2_32 -lgdi32 -lcomctl32 -static-libgcc -static-libstdc++ -Wno-deprecated-declarations
if %errorlevel% neq 0 (
    echo Build Failed!
    pause
    exit /b %errorlevel%
)
echo Build Successful! ChatSuite.exe created.
pause
