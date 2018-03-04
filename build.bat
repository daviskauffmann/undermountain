@echo off

set APP_NAME=roguelike

set SRC_DIR=src\*.c

set COMPILER_OPTS=/Fdbuild\ /Febin\%APP_NAME%.exe /Fobuild\ /Iinclude /W4 /Zi
set LINKER_OPTS=/libpath:lib /subsystem:console
set LIBS=libtcod-gui-VS.lib libtcod-VS.lib

del bin\*.exe
del bin\*.exp
del bin\*.ilk
del bin\*.lib
del bin\*.pdb

rd build /s /q 
md build

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
