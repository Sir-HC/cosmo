@echo off
cls

set CoreFlags=-DCORE_INTERNAL=1 -DPERFORMANCE_SLOW=1 -DPLATFORM_WIN32=1
set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4996 -wd4201 -wd4100 -wd4189 -FC -Z7
set CommonLinkerFlags= -opt:ref  user32.lib gdi32.lib winmm.lib
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl  %CommonCompilerFlags% ..\reboot\code\win32_core.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
cl  %CommonCompilerFlags% %CoreFlags% ..\code\core.cpp -Fmcore.map /LD /link -incremental:no /PDB:core_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
cl  %CommonCompilerFlags% %CoreFlags% ..\code\win32_core.cpp -Fmwin32_core.map /link %CommonLinkerFlags%
popd
