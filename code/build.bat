@echo off
cls

set CoreFlags=-DCORE_INTERNAL=1 -DPERFORMANCE_SLOW=1 -DPLATFORM_WIN32=1
set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4996 -wd4201 -wd4100 -wd4189 -FC -Fmwin32_core.map -Z7
set CommonLinkerFlags=-opt:ref  user32.lib gdi32.lib winmm.lib
IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM cl  %CommonCompilerFlags% ..\reboot\code\win32_core.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
cl  %CommonCompilerFlags% %CoreFlags% ..\reboot\code\win32_core.cpp /link %CommonLinkerFlags%
popd
