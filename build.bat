@echo off

set APP_NAME=roguelike

set SRC_DIR=src\*.c

set INCLUDE_DIRS=/Iinclude\libtcod-1.6.3 /Iinclude\SDL2-2.0.6
set LIB_DIRS=/libpath:lib\libtcod-1.6.3 /libpath:lib\SDL2-2.0.6
set LIBS=libtcod.lib libtcod-gui.lib SDL2.lib SDL2main.lib

set COMPILER_OPTS=/Fdpdb\ /Febin\%APP_NAME%.exe /Foobj\ /Zi
set LINKER_OPTS=/subsystem:windows

del bin\* /q
del obj\* /q
del pdb\* /q

cl %INCLUDE_DIRS% %COMPILER_OPTS% %SRC_DIR% /link %LIB_DIRS% %LINKER_OPTS% %LIBS%

copy resource\* .\bin

cd bin
%APP_NAME%.exe
cd ..\