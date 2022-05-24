@echo off

pushd %~dp0
cd ..
del /q .\out\*.*
rmdir /q .\out
mkdir out
cd out
cl ..\src\platform_win32\WinMain.cpp /nologo /std:c++20 /fp:contract /W4 /O2 /arch:AVX /MT /EHsc /link /subsystem:windows /out:Project256.exe  user32.lib 
rem cl ..\src\platform_win32\WinMain.cpp /EHsc /link /subsystem:windows user32.lib 