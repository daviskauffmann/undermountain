CC := gcc
CFLAGS := -ggdb -Iextern/libtcod/src -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
CPPFLAGS :=
LDFLAGS := -Lextern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
LDLIBS := -ltcod

SRC	:= \
	src/main.c \
	src/game/actor.c \
	src/game/assets.c \
	src/game/item.c \
	src/game/map.c \
	src/game/message.c \
	src/game/object.c \
	src/game/projectile.c \
	src/game/room.c \
	src/game/tile.c \
	src/game/util.c \
	src/game/world.c \
	src/platform/config.c \
	src/platform/platform.c \
	src/platform/sys.c \
	src/platform/scenes/scene_about.c \
	src/platform/scenes/scene_game.c \
	src/platform/scenes/scene_menu.c
OBJ := $(SRC:src/%.c=build/%.o)
DEP := $(OBJ:%.o=%.d)
TGT := bin/undermountain

.PHONY: all
all: $(TGT)

$(TGT): $(OBJ)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	cp extern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG/libtcod.dll bin

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) $(CPPFLAGS)

-include $(DEP)

.PHONY: run
run: all
	./$(TGT)

.PHONY: clean
clean:
	rm -rf bin build

.PHONY: build_libtcod
build_libtcod:
	cd extern/libtcod/buildsys/scons && scons build TOOLSET=mingw ARCH=x86_64

.PHONY: clean_libtcod
clean_libtcod:
	rm -rf extern/libtcod/buildsys/scons/__pycache__
	rm -rf extern/libtcod/buildsys/scons/dependencies
	rm -rf extern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
	rm -rf extern/libtcod/buildsys/scons/.sconsign.dblite
