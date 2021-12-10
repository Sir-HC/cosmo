@echo off
cls
mkdir ..\..\build
pushd ..\..\build
cl -FAsc -Zi ..\reboot\code\win32_core.cpp user32.lib gdi32.lib
popd