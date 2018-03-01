@echo off

set SRC_DIR=src\*.c

set COMPILER_OPTS=/Fdbuild\ /Febin\roguelike.exe /Fobuild\ /Iinclude /W4 /Zi
set LINKER_OPTS=/libpath:lib /subsystem:console
set LIBS=libtcod-gui-VS.lib libtcod-VS.lib

del bin\roguelike.exe
del bin\roguelike.exp
del bin\roguelike.ilk
del bin\roguelike.lib
del bin\roguelike.pdb

rd build /s /q 
md build

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
